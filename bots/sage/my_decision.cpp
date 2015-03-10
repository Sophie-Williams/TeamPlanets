// my_decision.cpp - MyDecision class implementation
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
#include "my_decision.hpp"

using namespace std;
using namespace team_planets;
using namespace sage;

MyDecision::MyDecision(const SageBot& bot, const Map& map):
  Decision(bot, map), num_ships_per_reinforcement_(10) {
}

MyDecision::~MyDecision() {
}

Decision::decisions_list MyDecision::generate_decisions() {
  // Perform frontline/backline classification
  perform_backline_frontline_classification_();

  // Compute the list of potential targets
  compute_list_of_potential_targets_();

  // Computing the list of possible decisions
  decisions_list decisions = recursively_generate_decisions_(frontline_planets(), potential_targets_);
  orders_list backline_orders = generate_backline_planets_orders_();
  for(orders_list& decision:decisions)
    decision.insert(decision.end(), backline_orders.begin(), backline_orders.end());

  return decisions;
}

// Generate a list of orders for all the ally planets
Decision::orders_list MyDecision::generate_allies_orders() const {
  orders_list orders;

  // Analyzing all the ally planets
  vector<planet_id> allied_frontline;
  vector<planet_id> allied_useless;

  for_each(map().planets_begin(), map().planets_end(),
           [this, &orders, &allied_frontline, &allied_useless](const Planet& planet) {
    if(bot().is_owned_by_my_team(planet) && !bot().is_owned_by_me(planet)) {
      vector<planet_id> neutral_neighbors;
      vector<planet_id> enemy_neighbors;

      for(size_t i = 0; i < bot().neighbors(planet.id()).size(); ++i) {
        const Planet& dst_planet = map().planet(bot().neighbors(planet.id())[i]);
        if(bot().is_neutral(dst_planet)) neutral_neighbors.push_back(dst_planet.id());
        if(bot().is_owned_by_enemy_team(dst_planet)) enemy_neighbors.push_back(dst_planet.id());
      }

      // If the planet have enemy neighbors, adding it to the list of potential reinforcements
      if(!enemy_neighbors.empty() || !neutral_neighbors.empty()) allied_frontline.push_back(planet.id());

      // Trying to attack the nearest enemy possible
      sort(enemy_neighbors.begin(), enemy_neighbors.end(),
           [this, &planet](const planet_id id1, const planet_id id2) {
        const unsigned int dist1 = planet.compute_travel_distance(map().planet(id1));
        const unsigned int dist2 = planet.compute_travel_distance(map().planet(id2));
        return dist1 < dist2;
      });

      bool    target_found  = false;
      size_t  target_idx    = 0;
      while(!target_found && target_idx < enemy_neighbors.size()) {
        const unsigned int num_ships = num_ships_to_take_a_planet_(planet.id(), enemy_neighbors[target_idx]);
        if(num_ships <= planet.current_num_ships()) target_found = true;
        else ++target_idx;
      }

      if(target_found) {
        orders.push_back(Fleet(neutral_player, planet.id(), enemy_neighbors[target_idx],
                               num_ships_to_take_a_planet_(planet.id(), enemy_neighbors[target_idx]), 0));
      } else {
        // If not, try to attack the nearest neutral
        sort(neutral_neighbors.begin(), neutral_neighbors.end(),
             [this, &planet](const planet_id id1, const planet_id id2) {
          const unsigned int dist1 = planet.compute_travel_distance(map().planet(id1));
          const unsigned int dist2 = planet.compute_travel_distance(map().planet(id2));
          return dist1 < dist2;
        });

        target_found = false;
        target_idx = 0;
        while(!target_found && target_idx < neutral_neighbors.size()) {
          const unsigned int num_ships =
              num_ships_to_take_a_planet_(planet.id(), neutral_neighbors[target_idx]);
          if(num_ships <= planet.current_num_ships()) target_found = true;
          else ++target_idx;
        }

        if(target_found) {
          orders.push_back(Fleet(neutral_player, planet.id(), neutral_neighbors[target_idx],
                                 num_ships_to_take_a_planet_(planet.id(), neutral_neighbors[target_idx]), 0));
        } else {
          // If still not, shuttle the ships to the less defended frontline planet
          if(planet.current_num_ships() >= num_ships_per_reinforcement_*planet.ship_increase())
            allied_useless.push_back(planet.id());
        }
      }
    }
  });

  // Creating the shuttle orders
  for(planet_id src_id:allied_useless) {
    planet_id best_frontline_to_reinforce = 0;
    unsigned int best_frontline_num_ships = 1000;
    for(planet_id dst_planet:allied_frontline) {
      if(map().planet(src_id).current_owner() == map().planet(dst_planet).current_owner()) {
        const unsigned int num_ships = map().planet(dst_planet).current_num_ships();
        if(best_frontline_to_reinforce == 0 || num_ships < best_frontline_num_ships) {
          best_frontline_to_reinforce = dst_planet;
          best_frontline_num_ships = num_ships;
        }
      }
    }

    if(best_frontline_to_reinforce != 0)
      orders.push_back(Fleet(neutral_player, src_id, best_frontline_to_reinforce,
                             map().planet(src_id).current_num_ships(), 0));
  }

  return orders;
}

void MyDecision::compute_list_of_potential_targets_() {
  // Clearing the list of potential targets
  potential_targets_.clear();

  // Analyzing the planets that can targeted by my frontline planets
  for(planet_id src_id:frontline_planets()) {
    // Analyzing each neighbor
    for(size_t i = 0; i < bot().neighbors(src_id).size(); ++i) {
      const Planet& dst_planet = map().planet(bot().neighbors(src_id)[i]);

      // Checking its status
      bool is_potential_target = bot().is_neutral(dst_planet)
                                 || (bot().team_is_complete() && bot().is_owned_by_enemy_team(dst_planet));

      // If the planet is targetable and it is not already targeted
      if(is_potential_target && !map().bot_planet_is_targeted_by_my_fleet(dst_planet.id())) {
        // Adding the planet to the list if it is not already in
        auto it = find_if(potential_targets_.begin(), potential_targets_.end(),
                          [&dst_planet](const planet_id& other_target) {
          return dst_planet.id() == other_target;
        });
        if(it == end(potential_targets_)) potential_targets_.push_back(dst_planet.id());
      }
    }
  }
}

// Generate a list of orders for the backline planets
Decision::orders_list MyDecision::generate_backline_planets_orders_() const {
  orders_list orders;

  for(planet_id src_id:backline_planets()) {
    if(map().planet(src_id).current_num_ships() > num_ships_per_reinforcement_*map().planet(src_id).ship_increase()) {
      // Searching for a frontline planet with the less ships on it
      planet_id     best_planet_to_reinforce    = 0;
      unsigned int  best_planet_num_ships       = 1000;
      for(planet_id dst_id:frontline_planets()) {
        const unsigned int num_ships = map().planet(dst_id).current_num_ships();
        if(best_planet_to_reinforce == 0 || num_ships < best_planet_num_ships) {
          best_planet_to_reinforce = dst_id;
          best_planet_num_ships = num_ships;
        }
      }

      // Sending reinforcements
      if(best_planet_to_reinforce != 0)
        orders.push_back(Fleet(neutral_player, src_id, best_planet_to_reinforce,
                               map().planet(src_id).current_num_ships(), 0));
    }
  }

  return orders;
}
