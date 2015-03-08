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
