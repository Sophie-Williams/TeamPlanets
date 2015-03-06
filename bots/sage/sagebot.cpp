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

#include "sagebot.hpp"

using namespace std;
using namespace team_planets;
using namespace sage;

void SageBot::perform_turn_() {
  for_each(map().planets_begin(), map().planets_end(), [this](const Planet& planet) {
    if(planet.current_owner() == map().myself()) {
      // For each of my planets
      if(planet.current_num_ships() > 10*planet.ship_increase()) {
        // The planet have enough ships to perform an attack

        // Finding the best planet to attack
        planet_id best_destination = 0;
        for_each(map().planets_begin(), map().planets_end(),
                 [this, &planet, &best_destination](const Planet& dest_planet) {
          if(dest_planet.current_owner() == neutral_player
              || !is_owned_by_my_team_(dest_planet)) {
            if(best_destination == 0) best_destination = dest_planet.id();
            else if(planet.compute_travel_distance(dest_planet)
                < planet.compute_travel_distance(map().planet(best_destination))) {
              best_destination = dest_planet.id();
            }
          }
        });

        // If the planet was found, attack!
        if(best_destination)
          map().bot_launch_fleet(planet.id(), best_destination, planet.current_num_ships());
      }
    }
  });
}
