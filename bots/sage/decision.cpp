// decision.cpp - Decision class implementation
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
#include "decision.hpp"

using namespace std;
using namespace team_planets;
using namespace sage;

Decision::Decision(const SageBot& bot, const Map& map):
  bot_(bot), map_(map) {
}

Decision::~Decision() {
}

Decision::decisions_list Decision::generate_decisions() {
  return decisions_list();
}

void Decision::perform_backline_frontline_classification_() {
  frontline_planets_.clear();
  backline_planets_.clear();

  for_each(map_.planets_begin(), map_.planets_end(), [this](const Planet& planet) {
    if(bot_.is_owned_by_me(planet)) {
      if(is_frontline_(planet.id())) frontline_planets_.push_back(planet.id());
      else backline_planets_.push_back(planet.id());
    }
  });
}

// Compute the number of ships needed to take a planet
unsigned int Decision::num_ships_to_take_a_planet_(planet_id src, planet_id dst) const {
  if(bot().is_neutral(map().planet(dst)))
    return map().planet(dst).current_num_ships() + 1;

  const unsigned int travel_dist = map().planet(dst).compute_travel_distance(map().planet(src));
  return map().planet(dst).current_num_ships() + travel_dist*map().planet(dst).ship_increase() + 1;
}

bool Decision::is_frontline_(planet_id id) const {
  bool frontline = false;

  for(size_t i = 0; i < bot_.neighbors(id).size(); ++i) {
    const Planet& planet = map_.planet(bot_.neighbors(id)[i]);
    if(bot_.is_neutral(planet) || bot_.is_owned_by_enemy_team(planet)) frontline = true;
  }

  return frontline;
}

// Recursive orders generation common for my and enemy decision
Decision::decisions_list Decision::recursively_generate_decisions_(const planets_list& sources,
                                                                   const planets_list& destinations) const {
  // Initializing the output decisions list
  decisions_list  decisions;
  decisions.push_back(orders_list()); // We have always the choice of doing nothing

  // Initializing the stack
  typedef stack<DecisionState_, vector<DecisionState_> >  decision_stack;
  decision_stack  S;
  S.push(generate_initial_decision_state_(sources, destinations));

  // Main loop (we arbitrary limit the maximum number of decisions because in some rare cases there is too much of
  // them)
  while(!S.empty() && decisions.size() < 50) {
    // Retrieving the current state
    DecisionState_ current_state = S.top();
    S.pop();

    // Generating toplevel orders for this state
    orders_list toplevel_orders = generate_toplevel_orders_for_a_given_state_(sources, current_state);

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
                        [&order](const planet_id& target) {
        return target == order.destination();
      });
      suborders_state.potential_targets.erase(it);

      // Updating the count of remaining ships
      suborders_state.remaining_ships = current_state.remaining_ships;

      size_t idx = 0;
      while(sources[idx] != order.source()) ++idx;
      suborders_state.remaining_ships[idx] -= order.num_ships();

      // Put the new state to the stack
      S.push(suborders_state);
    }
  }

  return decisions;
}

Decision::DecisionState_ Decision::generate_initial_decision_state_(const planets_list& sources,
                                                                    const planets_list& destinations) const {
  DecisionState_ state;
  state.potential_targets = destinations;

  // Filling in initial number of ships on sources planets
  state.remaining_ships.resize(sources.size(), 0);
  for(size_t i = 0; i < sources.size(); ++i)
    state.remaining_ships[i] = map().planet(sources[i]).current_num_ships();

  return state;
}

Decision::orders_list Decision::generate_toplevel_orders_for_a_given_state_(const planets_list& sources,
                                                                            const DecisionState_& state) const {
  orders_list output_orders;

  // Trying to target each potential target
  for(const planet_id target:state.potential_targets) {
    // Finding potential sources
    vector<size_t> src_planets_idx;
    for(size_t i = 0; i < sources.size(); ++i) {
      const planet_id src_id = sources[i];

      auto it = find_if(bot_.neighbors(src_id).begin(), bot_.neighbors(src_id).end(),
                        [&target](const planet_id neighbor_id) {
        return neighbor_id == target;
      });
      if(it != end(bot_.neighbors(src_id))) src_planets_idx.push_back(i);
    }

    // Generating the attack order, if the number of ships on the source planet remains positive
    for(size_t i = 0; i < src_planets_idx.size(); ++i) {
      const int remaining_ships = (int)state.remaining_ships[src_planets_idx[i]]
                                  - (int)num_ships_to_take_a_planet_(sources[src_planets_idx[i]], target);
      if(remaining_ships >= 0) {
        output_orders.push_back(Fleet(neutral_player, sources[src_planets_idx[i]], target,
                                      num_ships_to_take_a_planet_(sources[src_planets_idx[i]], target), 0));
      }
    }
  }

  return output_orders;
}
