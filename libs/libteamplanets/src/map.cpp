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
#include <algorithm>
#include "map.hpp"

using namespace std;
using namespace team_planets;

// Map loading functions
void Map::reset() {
  planets_.clear();
  fleets_.clear();
}

void Map::load(const string& file_name) {
  ifstream in(file_name);
  if(!in) throw runtime_error("Unable to load map from " + file_name + ".");

  // Loading planets in order
  planets_list tmp_list;
  while(in) {
    string tag;
    in >> tag;

    if(tag == string("P")) {
      Planet P;
      in >> P;

      assert(P.id() != 0);
      tmp_list.push_back(P);
    }
  }

  // Placing planets at correct positions (even if there is holes in planet's IDs)
  planet_id max_id = 0;
  for_each(tmp_list.begin(), tmp_list.end(), [&max_id](const Planet& P) {
    if(P.id() > max_id) max_id = P.id();
  });

  planets_.resize(max_id);
  for_each(tmp_list.begin(), tmp_list.end(), [this](const Planet& P) {
    planets_[P.id() - 1] = P;
  });
}

// Game mechanics for bot
void Map::bot_begin_turn() {
  // Clear the map before update
  planets_.clear();
  pending_orders_.clear();

  // Reading the input from the engine
  read_bot_input_();

  // Updating game status
  update_fleets_();
  remove_arrived_fleets_();
}

void Map::bot_end_turn() {
  write_bot_output_();
}

void Map::bot_launch_fleet(planet_id source, planet_id destination, unsigned int num_ships) {
  // Perform the launch
  planet(source).remove_ships(num_ships);
  fleets_.push_back(Fleet(myself_, source, destination, num_ships,
                          planet(destination).compute_travel_distance(planet(source))));

  // Store the pending order
  pending_orders_.push_back(Fleet(myself_, source, destination, num_ships,
                            planet(destination).compute_travel_distance(planet(source))));
}

// Game mechanics for engine
void Map::engine_launch_fleet(player_id player, planet_id source, planet_id destination, unsigned int num_ships) {
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
  planet(source).remove_ships(num_ships);
  fleets_.push_back(Fleet(player, source, destination, num_ships,
                          planet(destination).compute_travel_distance(planet(source))));
}

// Private common game mechanics
void Map::update_fleets_() {
  for_each(fleets_.begin(), fleets_.end(), [](Fleet& fleet) {
    fleet.advance();
  });
}

void Map::remove_arrived_fleets_() {
  fleet_iterator new_end = remove_if(fleets_.begin(), fleets_.end(), [](const Fleet& fleet) {
    return fleet.remaining_turns() == 0;
  });
  fleets_.erase(new_end, fleets_.end());
}

// Private bot game mechanics
void Map::read_bot_input_() {
  string tag;
  planets_list tmp_list;

  do {
    cin >> tag;

    if(tag == string("P")) {
      // Planet description line
      Planet P;
      cin >> P;
      tmp_list.push_back(P);
    }

    if(tag == string("M")) {
      // Message from the team
      cin >> message_;
    }

    if(tag == string("Y")) {
      // Current player identifier
      cin >> myself_;
    }
  } while(tag != string("."));

  // Placing planets at correct positions (even if there is holes in planet's IDs)
  planet_id max_id = 0;
  for_each(tmp_list.begin(), tmp_list.end(), [&max_id](const Planet& P) {
    if(P.id() > max_id) max_id = P.id();
  });

  planets_.resize(max_id);
  for_each(tmp_list.begin(), tmp_list.end(), [this](const Planet& P) {
    planets_[P.id() - 1] = P;
  });
}

void Map::write_bot_output_() {
  // Writing pending orders
  for_each(pending_orders_.begin(), pending_orders_.end(), [](const Fleet& fleet) {
    cout << fleet << endl;
  });

  // Writing the message for the team
  cout << "M " << message_ << endl;

  cout << "." << endl;
  cout.flush();
}
