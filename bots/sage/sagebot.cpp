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
  // Initialize decision tree root
  Leaf_ decision_root;
  decision_root.map = map();
  decision_root.current_player = Myself;
  decision_root.score = 0.0f;

  starting_time_ = chrono::high_resolution_clock::now();
  generate_decisions_(decision_root);

  // Classifying planets as front/back line
//  frontline_planets_.clear();
//  backline_planets_.clear();
//  for_each(map().planets_begin(), map().planets_end(), [this](const Planet& planet) {
//    if(is_owned_by_me_(planet)) {
//      if(is_frontline_(planet.id())) frontline_planets_.push_back(planet.id());
//      else backline_planets_.push_back(planet.id());
//    }
//  });
//
//  LOG << "frontline planets = ";
//  for(planet_id id:frontline_planets_) LOG << id << " ";
//  LOG << endl << "backline planets = ";
//  for(planet_id id:backline_planets_) LOG << id << " ";
//  LOG << endl << endl;
//
//  // Processing frontline planets
//  //for(planet_id id:frontline_planets_) process_frontline_planet_(id);
//  take_attack_decisions_();
//  LOG << endl;
//
//  // Processing backline planets
//  for(planet_id id:backline_planets_) process_backline_planet_(id);
//  LOG << endl;
}

// Classify a planet as front/back line
bool SageBot::is_frontline_(planet_id id) const {
  bool frontline = false;

  for(size_t i = 0; i < neighbors(id).size(); ++i) {
    const Planet& planet = map().planet(neighbors(id)[i]);
    if(is_neutral(planet) || is_owned_by_enemy_team(planet)) frontline = true;
  }

  return frontline;
}

// Compute the number of ships needed to take a planet
unsigned int SageBot::num_ships_to_take_a_planet_(planet_id src, planet_id dst) const {
  if(is_neutral(map().planet(dst)))
    return map().planet(dst).current_num_ships() + 1;

  const unsigned int travel_dist = map().planet(dst).compute_travel_distance(map().planet(src));
  return map().planet(dst).current_num_ships() + travel_dist*map().planet(dst).ship_increase() + 1;
}

// Computes the possible attack decisions
void SageBot::take_attack_decisions_() {
  struct Target {
    planet_id id;
    float     score;
  };
  vector<Target> potential_targets;

  // Computing the list of potential targets
  for(planet_id id:frontline_planets_) {
    for(size_t i = 0; i < neighbors(id).size(); ++i) {
      const Planet& dst_planet = map().planet(neighbors(id)[i]);

      Target tgt;
      tgt.id = dst_planet.id();
      tgt.score = (1.0f/(float)(dst_planet.current_num_ships() + 1))*(float)dst_planet.ship_increase();

      bool is_potential_target = is_neutral(dst_planet)
          || (team_is_complete() && is_owned_by_enemy_team(dst_planet));

      if(is_potential_target && !map().bot_planet_is_targeted_by_a_fleet(dst_planet.id())) {
        auto it = find_if(potential_targets.begin(), potential_targets.end(), [&tgt](const Target& target) {
          return target.id == tgt.id;
        });
        if(it == end(potential_targets)) potential_targets.push_back(tgt);
      }
    }
  }

  // Sorting them by score in decending order
  sort(potential_targets.begin(), potential_targets.end(), [](const Target& tgt1, const Target& tgt2) {
    return tgt1.score > tgt2.score;
  });

  for_each(potential_targets.begin(), potential_targets.end(), [this](const Target& target) {
    LOG << "Target planet " << target.id << ": owner = " << map().planet(target.id).current_owner();
    LOG << ", score = " << target.score << endl;
  });
  LOG << endl;

  // Trying to target each potential target
  for(const Target tgt:potential_targets) {
    // Finding potential sources
    vector<planet_id> src_planets;
    for(planet_id id:frontline_planets_) {
      auto it = find_if(neighbors(id).begin(), neighbors(id).end(), [&tgt](const planet_id nid) {
        return nid == tgt.id;
      });
      if(it != end(neighbors(id))) src_planets.push_back(id);
    }

    // Finding the one having the more ships after attacking this target
    size_t        best_source = 0;
    int           best_remaining_ships = (int)map().planet(src_planets[best_source]).current_num_ships()
                                         - (int)num_ships_to_take_a_planet_(src_planets[best_source], tgt.id);
    for(size_t i = 1; i < src_planets.size(); ++i) {
      const int remaining_ships = (int)map().planet(src_planets[i]).current_num_ships()
                                  - (int)num_ships_to_take_a_planet_(src_planets[i], tgt.id);
      if(best_remaining_ships < remaining_ships) {
        best_source = i;
        best_remaining_ships = remaining_ships;
      }
    }

    // Attacking the target, if possible
    if(best_remaining_ships > 0) {
      const unsigned int num_ships_to_send = num_ships_to_take_a_planet_(src_planets[best_source], tgt.id);

      map().bot_launch_fleet(src_planets[best_source], tgt.id, num_ships_to_send);
      LOG << "Attacking planet " << tgt.id << " from " << src_planets[best_source];
      LOG << " with " << num_ships_to_send << " ships." << endl;
    } else LOG << "Can't attack the planet " << tgt.id << ". Not enough ships." << endl;
  }
}

// Make a decision for a backline planet
void SageBot::process_backline_planet_(planet_id id) {
  LOG << "Backline planet " << id << ": ";

  if(map().planet(id).current_num_ships() > num_ships_per_reinforcement_*map().planet(id).ship_increase()) {
    // Searching for a nearest frontline planet
    planet_id     best_planet_to_reinforce    = 0;
    unsigned int  best_planet_travel_distance = 1000;
    for(planet_id dst_id:frontline_planets_) {
      const unsigned int dist = map().planet(dst_id).compute_travel_distance(map().planet(id));
      if(best_planet_to_reinforce == 0 || dist < best_planet_travel_distance) {
        best_planet_to_reinforce = dst_id;
        best_planet_travel_distance = dist;
      }
    }

    // Sending reinforcements
    if(best_planet_to_reinforce != 0) {
      LOG << "sending reinforcements of " << map().planet(id).current_num_ships() << " ships to the planet "
          << best_planet_to_reinforce << "." << endl;
      map().bot_launch_fleet(id, best_planet_to_reinforce, map().planet(id).current_num_ships());
    } else LOG << "no potential target, no further action." << endl;
  } else LOG << "not enough ships, no further action." << endl;
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

// Generate a list of possible decisions for a given decision leaf
void SageBot::generate_decisions_(Leaf_& leaf) const {
  Decision* decision = nullptr;
  if(leaf.current_player == Myself) decision = new MyDecision(*this, leaf.map);
  else decision = new EnemyDecision(*this, leaf.map);

  Decision::decisions_list decisions = decision->generate_decisions();

  delete decision;
}
