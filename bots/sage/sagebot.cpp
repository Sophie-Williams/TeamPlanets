// sagebot.cpp - SageBot class implementation
// sage - A TeamPlanets bot written for MachineZone job application
//
// Copyright (c) 2015 Vadim Litvinov <vadim_litvinov@fastmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the author nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

#include <stack>
#include <algorithm>
#include "log.hpp"
#include "my_decision.hpp"
#include "enemy_decision.hpp"
#include "sagebot.hpp"

using namespace std;
using namespace team_planets;
using namespace sage;

void SageBot::init_() {
  LOG << "SageBot Version 1.0 was started for player " << map().myself() << endl;
  LOG << "Perform initial computations..." << endl;

  // Computing mean travel distance between the planets
  planets_mean_distance_ = compute_planets_mean_distance_();
  LOG << "\tplanets_mean_distance = " << planets_mean_distance_ << endl;

  // Precomputing planets neighborhoods
  bool ok = false;
  neighborhood_radius_multiplier_ = 0;

  while(!ok) {
    ++neighborhood_radius_multiplier_;
    neighborhood_radius_ = neighborhood_radius_multiplier_*planets_mean_distance_;
    LOG << "\tneighborhood_radius = " << neighborhood_radius_ << endl;
    LOG << "\tPrecomputing planets neighborhoods..." << endl;
    compute_planets_neighborhoods_();

    ok = true;
    for(planet_id id = 1; id <= map().num_planets(); ++id) {
      if(neighbors(id).size() < 2) ok = false;
    }
  }

  for(planet_id id = 1; id <= map().num_planets(); ++id) {
    LOG << "\t\t" << id << " -> ";
    for(size_t i = 0; i < neighborhoods_[id - 1].size(); ++i) LOG << neighborhoods_[id - 1][i] << " ";
    LOG << endl;
  }

  LOG << endl;
}

void SageBot::perform_turn_() {
  // Initialize possibilities tree root
  Leaf_ root;
  root.current_turn = current_turn(); // The root is the previous turn
  root.map = map();
  root.current_player = Myself;       // To correctly start the process
  root.score = 0.0f;

  // Generating the tree of possibilities
  LOG << "Generating possibilities tree..." << endl;
  generate_possibilities_tree_(root);
  LOG << "Done in " << current_tree_gen_duration_().count() << " ms., depth = " << max_tree_depth_ << endl;

  // Computing scores of each possible move
  LOG << "Computing score for each move..." << endl;
  compute_possibility_tree_scores_(root);
  LOG << "Done." << endl;

  for(size_t i = 0; i < root.childrens.size(); ++i) {
    LOG << "Solution " << i << " with score = " << root.childrens[i].score << ": " << endl;
    for(const Fleet& fleet:root.childrens[i].orders)
      LOG << "\t" << fleet << endl;
  }

  if(!root.childrens.empty()) {
    size_t best_solution = 0;
    float best_solution_score = root.childrens[0].score;

    for(size_t i = 0; i < root.childrens.size(); ++i)
      if(best_solution_score < root.childrens[i].score) {
        best_solution = i;
        best_solution_score = root.childrens[i].score;
      }

    for(const Fleet& fleet:root.childrens[best_solution].orders)
      map().bot_launch_fleet(fleet.source(), fleet.destination(), fleet.num_ships());
  } else LOG << "No solutions!" << endl;
}

// Compute the mean distance between each pair of nearest planets in number of turns
unsigned int SageBot::compute_planets_mean_distance_() const {
  unsigned long long int dist_sum = 0;

  for_each(map().planets_begin(), map().planets_end(), [this, &dist_sum](const Planet& planet) {
    unsigned int distance_to_nearest_planet = 1000;
    for_each(map().planets_begin(), map().planets_end(), [&planet,&distance_to_nearest_planet](const Planet& planet2) {
      if(planet2.id() != planet.id()) {
        const unsigned int dist = planet.compute_travel_distance(planet2);
        if(dist < distance_to_nearest_planet) distance_to_nearest_planet = dist;
      }
    });

    dist_sum += distance_to_nearest_planet;
  });

  return (unsigned int)(dist_sum/(unsigned long long int)map().num_planets());
}

// Precompute planets neighborhoods
void SageBot::compute_planets_neighborhoods_() {
  neighborhoods_.clear();
  neighborhoods_.resize(map().num_planets());

  for(planet_id id = 1; id <= map().num_planets(); ++id) {
    for(planet_id id2 = 1; id2 <= map().num_planets(); ++id2) {
      if(id2 != id) {
        if(map().planet(id).compute_travel_distance(map().planet(id2)) <= neighborhood_radius_)
          neighborhoods_[id - 1].push_back(id2);
      }
    }
  }
}

// Generate the tree of possibilities
void SageBot::generate_possibilities_tree_(Leaf_& root) {
  // Initializing the process
  vector<Leaf_*>  current_level;
  vector<Leaf_*>  next_level;
  current_level.push_back(&root);

  // Main loop
  start_time_ = chrono::high_resolution_clock::now();
  chrono::milliseconds cur_duration(0);
  max_tree_depth_ = 0;
  while(!current_level.empty() && cur_duration < max_tree_comp_duration_) {
    next_level.clear();

    for(size_t n = 0; n < current_level.size() && cur_duration < max_tree_comp_duration_; ++n) {
      Leaf_& current_leaf = *(current_level[n]);

      // Checking if the current leaf is not an end game position
      if(!is_game_over_(current_leaf)) {
        // Generating childrens
        generate_possible_turns_(current_leaf);
        generate_enemy_turns_(current_leaf);

        // Updating the children maps
        update_child_leaves_maps(current_leaf);

        // Creating the next level
        for(size_t i = 0; i < current_leaf.childrens.size(); ++i)
          for(size_t j = 0; j < current_leaf.childrens[i].childrens.size(); ++j) {
            next_level.push_back(&(current_leaf.childrens[i].childrens[j]));
          }
      }

      cur_duration = current_tree_gen_duration_();
    }

    current_level.swap(next_level);
    ++max_tree_depth_;
    cur_duration = current_tree_gen_duration_();
  }
}

void SageBot::generate_possible_turns_(Leaf_& leaf) const {
  // Generating possible decisions
  MyDecision decision(*this, leaf.map);
  Decision::decisions_list posibilities = decision.generate_decisions();

  // Generating child leaves
  for(Decision::orders_list& posibility:posibilities) {
    leaf.childrens.emplace_back();
    Leaf_& new_leaf = leaf.childrens.back();

    new_leaf.current_turn = leaf.current_turn;
    new_leaf.current_player = Enemy;
    new_leaf.orders = posibility;
  }
}

void SageBot::generate_enemy_turns_(Leaf_& leaf) const {
  // Generating possible decisions
  EnemyDecision decision(*this, leaf.map);
  Decision::decisions_list posibilities = decision.generate_decisions();

  // Updating child leaves with enemy moves
  for(size_t i = 0; i < leaf.childrens.size(); ++i) {
    Leaf_& child = leaf.childrens[i];

    for(size_t j = 0; j < posibilities.size(); ++j) {
      Decision::orders_list& posibility = posibilities[j];

      child.childrens.emplace_back();
      Leaf_& new_leaf = child.childrens.back();

      new_leaf.current_turn = leaf.current_turn + 1;
      new_leaf.current_player = Myself;

      // Updating the map
      new_leaf.map = leaf.map;
      for(const Fleet& fleet:child.orders)
        new_leaf.map.bot_launch_fleet(fleet.source(), fleet.destination(), fleet.num_ships());
      for(const Fleet& fleet:posibility)
        new_leaf.map.bot_launch_fleet(fleet.source(), fleet.destination(), fleet.num_ships());
    }
  }
}

void SageBot::update_child_leaves_maps(Leaf_& leaf) const {
  for(size_t i = 0; i < leaf.childrens.size(); ++i) {
    Leaf_& child = leaf.childrens[i];
    for(size_t j = 0; j < child.childrens.size(); ++j) {
      Leaf_& current_leaf = child.childrens[j];
      current_leaf.map.engine_perform_turn();
    }
  }
}

bool SageBot::is_game_over_(const Leaf_& leaf) const {
  if(leaf.current_turn >= max_turn_) return true;

  unsigned int my_team_planets = 0;
  unsigned int enemy_team_planets = 0;
  for_each(leaf.map.planets_begin(), leaf.map.planets_end(),
           [this, &my_team_planets, &enemy_team_planets](const Planet& planet) {
    if(is_owned_by_my_team(planet)) ++my_team_planets;
    if(is_owned_by_enemy_team(planet)) ++enemy_team_planets;
  });

  return (my_team_planets == 0) || (enemy_team_planets == 0);
}

chrono::milliseconds SageBot::current_tree_gen_duration_() const {
  chrono::time_point<chrono::high_resolution_clock> cur_time = chrono::high_resolution_clock::now();
  return chrono::duration_cast<chrono::milliseconds>(cur_time - start_time_);
}

void SageBot::compute_possibility_tree_scores_(Leaf_& root) const {
  // Initializing stack
  struct State {
    Leaf_*  leaf;
    bool    second_pass;
  };
  typedef stack<State, vector<State> > state_stack;
  state_stack S;

  // Creating initial state
  State initial_state;
  initial_state.leaf = &root;
  initial_state.second_pass = false;
  S.push(initial_state);

  // Main loop
  while(!S.empty()) {
    // Retrieving current state
    State current_state = S.top();
    S.pop();

    if(current_state.leaf->childrens.empty()) {
      // This is a final state, computing its score
      current_state.leaf->score = compute_final_state_score_(*current_state.leaf);
    } else {
      if(current_state.second_pass) {
        // The scores of children was already computed
        current_state.leaf->score = 0.0f;
        for(const Leaf_& child:current_state.leaf->childrens)
          current_state.leaf->score += child.score;
        current_state.leaf->score /= (float)current_state.leaf->childrens.size();
      } else {
        // We need to compute the score of the childrens first
        current_state.second_pass = true;
        S.push(current_state);

        for(Leaf_& child:current_state.leaf->childrens) {
          State child_state;
          child_state.leaf = &child;
          child_state.second_pass = false;
          S.push(child_state);
        }
      }
    }
  }
}

float SageBot::compute_final_state_score_(const Leaf_& leaf) const {
  const float planet_coeff = 0.1f;
  const float ship_coeff = 0.0001f;

  // Evaluating the number of ships and planets for each team
  unsigned int my_team_planets = 0, my_team_ships = 0;
  unsigned int enemy_team_planets = 0, enemy_team_ships = 0;
  unsigned int neutral_planets = 0;

  for_each(leaf.map.planets_begin(), leaf.map.planets_end(),
           [this, &my_team_planets, &my_team_ships, &enemy_team_planets,
            &enemy_team_ships, &neutral_planets](const Planet& planet) {
    if(is_neutral(planet)) ++neutral_planets;
    else {
      if(is_owned_by_my_team(planet)) {
        ++my_team_planets;
        my_team_ships += planet.current_num_ships();
      } else if(is_owned_by_enemy_team(planet)) {
        ++enemy_team_planets;
        enemy_team_ships += planet.current_num_ships();
      }
    }
  });

  if(my_team_planets == 0) return -1000.0f;    // We are dead, very bad
  if(enemy_team_planets == 0) return 1000.0f;  // Enemy is dead, very good
  return (float)my_team_ships*ship_coeff + (float)my_team_planets*planet_coeff
         - (float)enemy_team_ships*ship_coeff - (float)enemy_team_planets*planet_coeff;
}
