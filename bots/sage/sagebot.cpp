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

  if(!root.childrens.empty()) {
    size_t max_solution = 0;
    size_t max_solution_size = root.childrens[0].orders.size();
    for(size_t i = 0; i < root.childrens.size(); ++i)
      if(max_solution_size < root.childrens[i].orders.size()) {
        max_solution = i;
        max_solution_size = root.childrens[i].orders.size();
      }

    for(const Fleet& fleet:root.childrens[max_solution].orders)
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
  // Initializing stack
  typedef std::stack<Leaf_*, std::vector<Leaf_*> > leaves_stack;
  leaves_stack S;
  S.push(&root);

  // Main loop
  start_time_ = chrono::high_resolution_clock::now();
  max_tree_depth_ = root.current_turn;
  bool time_stop = false;

  while(!S.empty() && !time_stop) {
    // Extracting current leaf
    Leaf_& current_leaf = *(S.top());
    S.pop();
    if(current_leaf.current_turn > max_tree_depth_) max_tree_depth_ = current_leaf.current_turn;

    // Checking if the current leaf is not an end game position
    if(!is_game_over_(current_leaf)) {
      // Generating childrens
      time_stop = generate_possible_turns_(current_leaf);
      if(time_stop) {
        current_leaf.childrens.clear();
        continue;
      }

      // Generate enemy responses
      time_stop = generate_enemy_turns_(current_leaf);
      if(time_stop) {
        current_leaf.childrens.clear();
        continue;
      }

      // Updating stack, if the computation time is not over
      for(size_t i = 0; i < current_leaf.childrens.size() && !time_stop; ++i)
        for(size_t j = 0; j < current_leaf.childrens[i].childrens.size() && !time_stop; ++j) {
          if(i == 0 && j == 0) continue;  // First child is a no-op, ignore it!

          // Checking current computation time
          const chrono::milliseconds cur_duration = current_tree_gen_duration_();
          if(cur_duration < max_tree_comp_duration_) S.push(&(current_leaf.childrens[i].childrens[j]));
          else time_stop = true;
        }
    }
  }

  max_tree_depth_ -= current_turn();
}

bool SageBot::generate_possible_turns_(Leaf_& leaf) const {
  // Generating possible decisions
  MyDecision decision(*this, leaf.map);
  Decision::decisions_list posibilities = decision.generate_decisions();

  // Generating child leaves
  bool time_stop = false;
  for(Decision::orders_list& posibility:posibilities) {
    const chrono::milliseconds cur_duration = current_tree_gen_duration_();
    if(cur_duration < max_tree_comp_duration_) {
      leaf.childrens.emplace_back();
      Leaf_& new_leaf = leaf.childrens.back();

      new_leaf.current_turn = leaf.current_turn;
      new_leaf.current_player = Enemy;
      new_leaf.orders = posibility;
    } else {
      time_stop = true;
      break;
    }
  }

  return time_stop;
}

bool SageBot::generate_enemy_turns_(Leaf_& leaf) const {
  // Generating possible decisions
  EnemyDecision decision(*this, leaf.map);
  Decision::decisions_list posibilities = decision.generate_decisions();

  // Updating child leaves with enemy moves
  bool time_stop = false;
  for(size_t i = 0; i < leaf.childrens.size() && !time_stop; ++i) {
    Leaf_& child = leaf.childrens[i];

    for(size_t j = 0; j < posibilities.size() && !time_stop; ++j) {
      Decision::orders_list& posibility = posibilities[j];

      const chrono::milliseconds cur_duration = current_tree_gen_duration_();
      if(cur_duration < max_tree_comp_duration_) {
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
        new_leaf.map.engine_perform_turn();
      } else time_stop = true;
    }
  }

  return time_stop;
}

bool SageBot::is_game_over_(const Leaf_& leaf) const {
  if(leaf.current_turn >= 200) return true;

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
