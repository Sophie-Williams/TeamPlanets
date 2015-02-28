// map.cpp - Map class implementation
// libTeamPlanets - A library of common data structures for engine and bots
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

#include <iostream>
#include <fstream>
#include <stdexcept>
#include "map.hpp"

using namespace std;
using namespace team_planets;

// Map loading functions
void Map::reset() {
  planets_.clear();
  fleets_.clear();
}

void Map::load_google_ai_challenge_map(const string& file_name) {
  ifstream in(file_name);
  if(!in) throw runtime_error("Unable to load map from " + file_name + ".");

  while(in) {
    // Reading data
    string tag;
    float x, y;
    unsigned int start_player_id, start_num_ships, ship_incr;
    in >> tag >> x >> y >> start_player_id >> start_num_ships >> ship_incr;

    if(tag == string("P")) {
      planets_.push_back(Planet((planet_id)(planets_.size() + 1), Coordinates(x, y), ship_incr,
                                neutral_player, start_num_ships));
    }
  }
}

// Game mechanics implementation
void Map::launch_fleet(player_id player, planet_id source, planet_id destination, unsigned int num_ships) {
  // Perform the launch
  planet(source).remove_ships(num_ships);
  fleets_.push_back(Fleet(player, source, destination, num_ships,
                          planet(destination).compute_travel_distance(planet(source))));
}

// Game mechanics with validation (for the engine)
void Map::launch_fleet_with_validation(player_id player, planet_id source, planet_id destination,
                                       unsigned int num_ships) {
  // Performing logical checks
  if(source == 0) throw logic_error("Fleet launch: Invalid source planet.");
  if(source > planets_.size()) throw logic_error("Fleet launch: Invalid source planet.");
  if(destination == 0) throw logic_error("Fleet launch: Invalid source planet.");
  if(destination > planets_.size()) throw logic_error("Fleet launch: Invalid source planet.");
  if(planet(source).current_owner() != player)
    throw logic_error("Fleet launch: Player is not the owner of the source planet.");
  if(planet(source).current_num_ships() < num_ships)
    throw logic_error("Fleet launch: Source planet haven't enough ships.");

  // Performing the order
  launch_fleet(player, source, destination, num_ships);
}
