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
#include "log.hpp"
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
  LOG << "frontline planets = ";
  for(planet_id id:frontline_planets()) LOG << id << " ";
  LOG << endl << "backline planets = ";
  for(planet_id id:backline_planets()) LOG << id << " ";
  LOG << endl << endl;

  // Compute the list of potential targets
  compute_list_of_potential_targets_();

  // Computing the list of possible decisions
  decisions_list decisions = recursively_generate_decisions_();
  orders_list backline_orders = generate_backline_planets_orders_();
  for(orders_list& decision:decisions)
    decision.insert(decision.end(), backline_orders.begin(), backline_orders.end());

  return decisions;
}

void MyDecision::compute_list_of_potential_targets_() {
  // Clearing the list of potential targets
  potential_targets_.clear();

  // Analyzing the planets that can targeted by my frontline planets
  for(planet_id src_id:frontline_planets()) {
    // Analyzing each neighbor
    for(size_t i = 0; i < bot().neighbors(src_id).size(); ++i) {
      const Planet& dst_planet = map().planet(bot().neighbors(src_id)[i]);

      // Computing its score
      Target_ target;
      target.id = dst_planet.id();
      target.score = (1.0f/(float)(dst_planet.current_num_ships() + 1))*(float)dst_planet.ship_increase();

      // Checking its status
      bool is_potential_target = bot().is_neutral(dst_planet)
                                 || (bot().team_is_complete() && bot().is_owned_by_enemy_team(dst_planet));

      // If the planet is targetable and it is not already targeted
      if(is_potential_target && !map().bot_planet_is_targeted_by_a_fleet(dst_planet.id())) {
        // Adding the planet to the list if it is not already in
        auto it = find_if(potential_targets_.begin(), potential_targets_.end(), [&target](const Target_& other_target) {
          return target.id == other_target.id;
        });
        if(it == end(potential_targets_)) potential_targets_.push_back(target);
      }
    }
  }

  // Sorting the potential targets by score in descending order
  sort(potential_targets_.begin(), potential_targets_.end(), [](const Target_& target1, const Target_& target2) {
    return target1.score > target2.score;
  });
}

Decision::decisions_list MyDecision::recursively_generate_decisions_() const {
  // Initializing the output decisions list
  decisions_list  decisions;
  decisions.push_back(orders_list()); // We have always the choice of doing nothing

  // Initializing the stack
  typedef stack<DecisionState_, vector<DecisionState_> >  decision_stack;
  decision_stack  S;
  S.push(generate_initial_decision_state_());

  // Main loop
  while(!S.empty()) {
    // Retrieving the current state
    DecisionState_ current_state = S.top();
    S.pop();

    // Generating toplevel orders for this state
    orders_list toplevel_orders = generate_toplevel_orders_for_a_given_state_(current_state);

    // Generating decisions and the corresponding decision states for each toplevel order
    for(const Fleet& order:toplevel_orders) {
      DecisionState_ suborders_state;

      suborders_state.current_decision = current_state.current_decision;
      suborders_state.current_decision.push_back(order);

      // Current decision is a decision by itself, without any suborders
      decisions.push_back(suborders_state.current_decision);

      // The target of the current order is not a potential target anymore
      suborders_state.potential_targets = current_state.potential_targets;
      auto it = find_if(suborders_state.potential_targets.begin(), suborders_state.potential_targets.end(),
                        [&order](const Target_& target) {
        return target.id == order.destination();
      });
      suborders_state.potential_targets.erase(it);

      // Updating the count of remaining ships
      suborders_state.remaining_ships = current_state.remaining_ships;

      size_t idx = 0;
      while(frontline_planets()[idx] != order.source()) ++idx;

      suborders_state.remaining_ships[idx] -= order.num_ships();

      // Put the new state to the stack
      S.push(suborders_state);
    }
  }

  return decisions;
}

MyDecision::DecisionState_ MyDecision::generate_initial_decision_state_() const {
  DecisionState_ state;
  state.potential_targets = potential_targets_;

  // Filling in initial number of ships on frontline planets
  state.remaining_ships.resize(frontline_planets().size(), 0);
  for(size_t i = 0; i < frontline_planets().size(); ++i)
    state.remaining_ships[i] = map().planet(frontline_planets()[i]).current_num_ships();

  return state;
}

Decision::orders_list MyDecision::generate_toplevel_orders_for_a_given_state_(const DecisionState_& state) const {
  orders_list output_orders;

  // Trying to target each potential target
  for(const Target_ target:state.potential_targets) {
    // Finding potential sources
    vector<size_t> src_planets_idx;
    for(size_t i = 0; i < frontline_planets().size(); ++i) {
      const planet_id src_id = frontline_planets()[i];

      auto it = find_if(bot().neighbors(src_id).begin(), bot().neighbors(src_id).end(),
                        [&target](const planet_id neighbor_id) {
        return neighbor_id == target.id;
      });
      if(it != end(bot().neighbors(src_id))) src_planets_idx.push_back(i);
    }

    // Finding the one having the more ships after attacking this target
    size_t        best_source = 0;
    int           best_remaining_ships = (int)state.remaining_ships[src_planets_idx[best_source]]
                                         - (int)num_ships_to_take_a_planet_(
                                             frontline_planets()[src_planets_idx[best_source]], target.id);
    for(size_t i = 1; i < src_planets_idx.size(); ++i) {
      const int remaining_ships = (int)state.remaining_ships[src_planets_idx[i]]
                                  - (int)num_ships_to_take_a_planet_(frontline_planets()[src_planets_idx[i]],
                                                                     target.id);
      if(best_remaining_ships < remaining_ships) {
        best_source = i;
        best_remaining_ships = remaining_ships;
      }
    }

    // Generating the attack order, if the number of ships on the planet remains positive
    if(best_remaining_ships >= 0) {
      const unsigned int num_ships_to_send = num_ships_to_take_a_planet_(
                                               frontline_planets()[src_planets_idx[best_source]], target.id);
      output_orders.push_back(Fleet(neutral_player, frontline_planets()[src_planets_idx[best_source]],
                                    target.id, num_ships_to_send, 0));
    }
  }

  return output_orders;
}

// Generate a list of orders for the backline planets
Decision::orders_list MyDecision::generate_backline_planets_orders_() const {
  orders_list orders;

  for(planet_id src_id:backline_planets()) {
    if(map().planet(src_id).current_num_ships() > num_ships_per_reinforcement_*map().planet(src_id).ship_increase()) {
      // Searching for a nearest frontline planet
      planet_id     best_planet_to_reinforce    = 0;
      unsigned int  best_planet_travel_distance = 1000;
      for(planet_id dst_id:frontline_planets()) {
        const unsigned int dist = map().planet(dst_id).compute_travel_distance(map().planet(src_id));
        if(best_planet_to_reinforce == 0 || dist < best_planet_travel_distance) {
          best_planet_to_reinforce = dst_id;
          best_planet_travel_distance = dist;
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
