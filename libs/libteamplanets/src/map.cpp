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

bool Map::bot_planet_is_targeted_by_a_fleet(planet_id id) const {
  auto it = find_if(fleets_.begin(), fleets_.end(), [id](const Fleet& fleet) {
    return fleet.destination() == id;
  });
  return it != end(fleets_);
}

// Game mechanics for engine
void Map::engine_perform_turn() {
  update_fleets_();
  perform_battles_();
  remove_arrived_fleets_();
  update_planets_();
}

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

void Map::engine_eliminate_player_fleets(player_id player) {
  fleet_iterator new_end = remove_if(fleets_.begin(), fleets_.end(), [player](const Fleet& fleet) {
    return fleet.player() == player;
  });
  fleets_.erase(new_end, fleets_.end());
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

void Map::perform_battles_() {
  // Structure describing a force arrived at the planet
  struct Force {
    player_id     player;
    unsigned int  num_ships;
  };
  typedef vector<Force>       forces_list;
  typedef vector<forces_list> planetary_forces_list;

  // Initializing the list of forces
  planetary_forces_list forces_per_planet;
  forces_per_planet.resize(planets_.size());
  for(planet_id id = 1; id <= planets_.size(); ++id) {
    forces_list& forces = forces_per_planet[id - 1];
    forces.push_back(Force());
    forces.back().player = planets_[id - 1].current_owner();
    forces.back().num_ships = planets_[id - 1].current_num_ships();
  }

  // Classifying arrived fleets by planets and creating forces
  for(const Fleet& fleet:fleets_) {
    if(fleet.remaining_turns() == 0) {
      forces_list& forces = forces_per_planet[fleet.destination() - 1];

      // Searching player force
      Force* player_force = nullptr;
      bool   found        = false;
      for(Force& force:forces) {
        if(force.player == fleet.player()) {
          player_force = &force;
          found = true;
        }
      }

      // Updating the force accrodingly
      if(found) player_force->num_ships += fleet.num_ships();
      else {
        forces.push_back(Force());
        forces.back().player = fleet.player();
        forces.back().num_ships = fleet.num_ships();
      }
    }
  }

  // Performing battles
  for(planet_id id = 1; id <= forces_per_planet.size(); ++id) {
    Planet& planet = planets_[id - 1];
    forces_list& forces = forces_per_planet[id - 1];

    if(forces.size() > 1) {
      // Searching the largest and second largest force
      size_t max_force = 0, second_max = 1;
      for(size_t i = 1; i < forces.size(); ++i) {
        if(forces[i].num_ships >= forces[max_force].num_ships) {
          second_max = max_force;
          max_force = i;
        }
      }

      // If the forces are equal, current owner keeps the planet
      if(forces[max_force].num_ships == forces[second_max].num_ships) planet.set_current_num_ships(0);
      else {
        // Max force wins the planet
        planet.set_current_owner(forces[max_force].player);
        planet.set_current_num_ships(forces[max_force].num_ships - forces[second_max].num_ships);
      }
    } else {
      // The force returns to the planet
      planet.set_current_num_ships(forces[0].num_ships);
    }
  }
}

void Map::update_planets_() {
  for_each(planets_.begin(), planets_.end(), [](Planet& planet) {
    if(planet.current_owner() != neutral_player) planet.produce_new_ships();
  });
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
