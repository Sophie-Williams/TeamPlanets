// main.cpp - Application entry point
// bully - A TeamPlanets bots implementing a very simple aggressive strategy
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

#include <cstdlib>
#include <algorithm>
#include "map.hpp"

using namespace std;
using namespace team_planets;

int main(int argc, char* argv[]) {
  Map map;

  // The bot main loop
  while(true) {
    map.bot_begin_turn();

    // If we have a fleet in flight, do nothing
    if(map.num_fleets() == 0) {
      // Search one of my planets having the more ships on it
      planet_id source_planet = 0;
      for_each(map.planets_begin(), map.planets_end(), [&map, &source_planet](const Planet& planet) {
        if(planet.current_owner() == map.myself()) {
          if(source_planet == 0) source_planet = planet.id();
          else {
            if(map.planet(source_planet).current_num_ships() < planet.current_num_ships())
              source_planet = planet.id();
          }
        }
      });

      // Search one of enemy or neutral planets having the less ships on it
      planet_id destination_planet = 0;
      for_each(map.planets_begin(), map.planets_end(), [&map, &destination_planet](const Planet& planet) {
        if(planet.current_owner() != map.myself()) {
          if(destination_planet == 0) destination_planet = planet.id();
          else {
            if(map.planet(destination_planet).current_num_ships() > planet.current_num_ships())
              destination_planet = planet.id();
          }
        }
      });

      // Sending the fleet (half the number of ships on the source planet)
      map.bot_launch_fleet(source_planet, destination_planet, map.planet(source_planet).current_num_ships()/2);
    }

    map.bot_end_turn();
  }

  return EXIT_SUCCESS;
}
