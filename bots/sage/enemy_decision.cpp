// enemy_decision.cpp - EnemyDecision class implementation
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
#include "enemy_decision.hpp"

using namespace std;
using namespace team_planets;
using namespace sage;

EnemyDecision::EnemyDecision(const SageBot& bot, const Map& map):
  Decision(bot, map) {
}

EnemyDecision::~EnemyDecision() {
}

Decision::decisions_list EnemyDecision::generate_decisions() {
  // Perform frontline/backline classification
  perform_backline_frontline_classification_();

  // Compute the list of potential sources and targets
  compute_list_of_potential_sources_and_targets_();

  // Computing the list of possible decisions
  return recursively_generate_decisions_(potential_sources_, potential_targets_);
}

void EnemyDecision::compute_list_of_potential_sources_and_targets_() {
  // Clearing the list of potential sources and targets
  potential_sources_.clear();
  potential_targets_.clear();

  // Analyzing my frontline planets that can be targeted by the enemy
  for(planet_id dst_id:frontline_planets()) {
    bool is_potential_target = false;

    // Analyzing each neighbor
    for(size_t i = 0; i < bot().neighbors(dst_id).size(); ++i) {
      const Planet& src_planet = map().planet(bot().neighbors(dst_id)[i]);

      // If the planet is owned by the enemy
      if(bot().is_owned_by_enemy_team(src_planet)) {
        is_potential_target = true; // Our planet is a potential target for the enemy

        // Adding the planet to the list if it is not already in
        auto it = find_if(potential_sources_.begin(), potential_sources_.end(),
                          [&src_planet](const planet_id& other_planet) {
          return src_planet.id() == other_planet;
        });
        if(it == end(potential_sources_)) potential_sources_.push_back(src_planet.id());
      }
    }

    if(is_potential_target) potential_targets_.push_back(dst_id);
  }
}
