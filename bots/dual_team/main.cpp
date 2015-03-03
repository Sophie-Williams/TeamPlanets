// main.cpp - Application entry point
// dual_team - A TeamPlanets bots implementing a slightly more elaborated aggressive strategy (with team support)
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

bool is_owned_by_a_team(const vector<player_id>& team, const Planet& planet) {
  auto it = find(team.begin(), team.end(), planet.current_owner());
  return it != end(team);
}

int main(int argc, char* argv[]) {
  Map map;
  vector<player_id> my_team;
  size_t            teammate_id_to_send = 0;

  // The bot main loop
  while(true) {
    map.bot_begin_turn();

    // Update the list of team mates
    if(my_team.empty()) my_team.push_back(map.myself());
    if(map.message() != 0 && map.message() != (uint32_t)map.myself())
      my_team.push_back((player_id)map.message());

    // Determining the current strategy
    bool          attack_the_enemy          = false;
    unsigned int  max_simultaneous_attacks  = 1;

    // Counting mine and enemy resources
    unsigned int my_ships = 0, my_production = 0;
    unsigned int enemy_ships = 0, enemy_production = 0;
    for_each(map.planets_begin(), map.planets_end(),
             [&map, &my_team, &my_ships, &my_production, &enemy_ships, &enemy_production](const Planet& planet) {
      if(is_owned_by_a_team(my_team, planet)) {
        my_ships += planet.current_num_ships();
        my_production += planet.ship_increase();
      } else if(planet.current_owner() != neutral_player) {
        enemy_ships += planet.current_num_ships();
        enemy_production += planet.ship_increase();
      }
    });

    if(my_ships > enemy_ships) {
      if(my_production > enemy_production) {
        // I am in overwhelming advantage, conservatively sibling the enemy
        max_simultaneous_attacks = 1;
        attack_the_enemy = true;
      } else max_simultaneous_attacks = 3; // Expanding agressively
    } else {
      if(my_production > enemy_production) max_simultaneous_attacks = 1;
      else max_simultaneous_attacks = 5;
    }

    // If we have too much fleets in flight, do nothing
    if(map.num_fleets() <= max_simultaneous_attacks) {
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
      for_each(map.planets_begin(), map.planets_end(),
               [attack_the_enemy, &map, &my_team, &destination_planet](const Planet& planet) {
        if(planet.current_owner() == neutral_player || !is_owned_by_a_team(my_team, planet)) {
          if(!attack_the_enemy || planet.current_owner() != neutral_player) {
            if(destination_planet == 0) destination_planet = planet.id();
            else {
              if(map.planet(destination_planet).current_num_ships() > planet.current_num_ships())
                destination_planet = planet.id();
            }
          }
        }
      });

      // Sending the fleet (half the number of ships on the source planet)
      if(map.planet(source_planet).current_num_ships() > 1)
        map.bot_launch_fleet(source_planet, destination_planet, map.planet(source_planet).current_num_ships()/2);
    }

    // Generating the message to the team
    if(teammate_id_to_send < my_team.size()) {
      map.set_message((uint32_t)my_team[teammate_id_to_send]);
      ++teammate_id_to_send;
    } else map.set_message(0);

    map.bot_end_turn();
  }

  return EXIT_SUCCESS;
}
