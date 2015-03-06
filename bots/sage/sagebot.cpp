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
#include "log.hpp"
#include "sagebot.hpp"

using namespace std;
using namespace team_planets;
using namespace sage;

void SageBot::init_() {
  LOG << "SageBot Version 1.0 was started for player " << map().myself() << endl;
  LOG << "Perform initial computations..." << endl;

  // Computing mean travel distance between the planets
  planets_mean_distance_ = compute_planets_mean_distance_();
  LOG << "\tplanets_mean_distance = " << planets_mean_distance_ << endl;

  // Precomputing planets neighborhoods
  neighborhood_radius_ = neighborhood_radius_multiplier_*planets_mean_distance_;
  LOG << "\tneighborhood_radius = " << neighborhood_radius_ << endl;
  LOG << "\tPrecomputing planets neighborhoods..." << endl;
  compute_planets_neighborhoods_();

  for(planet_id id = 1; id <= map().num_planets(); ++id) {
    LOG << "\t\t" << id << " -> ";
    for(size_t i = 0; i < neighborhoods_[id - 1].size(); ++i) LOG << neighborhoods_[id - 1][i] << " ";
    LOG << endl;
  }

  LOG << endl;
}

void SageBot::perform_turn_() {
  // Classifying planets as front/back line
  frontline_planets_.clear();
  backline_planets_.clear();
  for_each(map().planets_begin(), map().planets_end(), [this](const Planet& planet) {
    if(is_owned_by_me_(planet)) {
      if(is_frontline_(planet.id())) frontline_planets_.push_back(planet.id());
      else backline_planets_.push_back(planet.id());
    }
  });

  LOG << "frontline planets = ";
  for(planet_id id:frontline_planets_) LOG << id << " ";
  LOG << endl << "backline planets = ";
  for(planet_id id:backline_planets_) LOG << id << " ";
  LOG << endl << endl;

  // Processing frontline planets
  for(planet_id id:frontline_planets_) process_frontline_planet_(id);
  LOG << endl;

  // Processing backline planets
  for(planet_id id:backline_planets_) process_backline_planet_(id);
  LOG << endl;
}

// Classify a planet as front/back line
bool SageBot::is_frontline_(planet_id id) const {
  bool frontline = false;

  for(size_t i = 0; i < neighbors_(id).size(); ++i) {
    const Planet& planet = map().planet(neighbors_(id)[i]);
    if(is_neutral_(planet) || is_owned_by_enemy_team_(planet)) frontline = true;
  }

  return frontline;
}

// Compute the number of ships needed to take a planet
unsigned int SageBot::num_ships_to_take_a_planet_(planet_id src, planet_id dst) const {
  if(is_neutral_(map().planet(dst)))
    return map().planet(dst).current_num_ships() + 1;

  const unsigned int travel_dist = map().planet(dst).compute_travel_distance(map().planet(src));
  return map().planet(dst).current_num_ships() + travel_dist*map().planet(dst).ship_increase() + 1;
}

// Compute a planet score to find if the planet is a useful target
float SageBot::compute_target_planet_score_(planet_id src, planet_id dst) const {
  const unsigned int num_ships = num_ships_to_take_a_planet_(src, dst);
  return (float)num_ships/(float)map().planet(dst).ship_increase();
}

// Make a decision for a frontline planet
void SageBot::process_frontline_planet_(planet_id id) {
  LOG << "Frontline planet " << id << ": ";

  // Computing a safe number of ships
//  unsigned int safe_number_of_ships = 0;
//  for(size_t i = 0; i < neighbors_(id).size(); ++i) {
//    if(is_owned_by_enemy_team_(map().planet(neighbors_(id)[i]))) {
//      const unsigned int ships = num_ships_to_take_a_planet_(neighbors_(id)[i], id);
//      if(ships > safe_number_of_ships) safe_number_of_ships = ships;
//    }
//  }

//  if(map().planet(id).current_num_ships() > safe_number_of_ships) {
    // Planet have enough ship to attack

    // Searching for a target planet
    planet_id best_target_planet  = 0;
    float     best_target_score   = 0.0f;
    for(size_t i = 0; i < neighbors_(id).size(); ++i) {
      const Planet& dst_planet = map().planet(neighbors_(id)[i]);

      // We only attack other players planets if we know all our allies
      // to not accidently kill one.
      bool is_targetable = false;
      if(is_neutral_(dst_planet)) is_targetable = true;
      else if(team_is_complete_() && is_owned_by_enemy_team_(dst_planet)) is_targetable = true;

      if(is_targetable && !map().bot_planet_is_targeted_by_a_fleet(dst_planet.id())) {
        const float score = compute_target_planet_score_(id, dst_planet.id());
        if(best_target_planet == 0 || best_target_score < score) {
          best_target_planet = dst_planet.id();
          best_target_score = score;
        }
      }
    }

    if(best_target_planet != 0) {
      const unsigned int num_ships_to_send = num_ships_to_take_a_planet_(id, best_target_planet);

      if(num_ships_to_send <= map().planet(id).current_num_ships() /* &&
         map().planet(id).current_num_ships() - num_ships_to_send > safe_number_of_ships */) {
        map().bot_launch_fleet(id, best_target_planet, num_ships_to_send);
        LOG << "targeting " << best_target_planet << " with " << num_ships_to_send << " ships." << endl;
      } else LOG << "targeting " << best_target_planet << " would put this planet in danger, no further action."
                 << endl;
    } else LOG << "no potential target, no further action." << endl;
//  } else LOG << "in danger, no further action." << endl;
}

// Make a decision for a backline planet
void SageBot::process_backline_planet_(planet_id id) {
  LOG << "Backline planet " << id << ": ";

  if(map().planet(id).current_num_ships() > num_ships_per_reinforcement_*map().planet(id).ship_increase()) {
    // Searching for a nearest frontline planet
    planet_id     best_planet_to_reinforce    = 0;
    unsigned int  best_planet_travel_distance = 1000;
    for(planet_id dst_id:frontline_planets_) {
      const unsigned int dist = map().planet(dst_id).compute_travel_distance(map().planet(id));
      if(best_planet_to_reinforce == 0 || dist < best_planet_travel_distance) {
        best_planet_to_reinforce = dst_id;
        best_planet_travel_distance = dist;
      }
    }

    // Sending reinforcements
    if(best_planet_to_reinforce != 0) {
      map().bot_launch_fleet(id, best_planet_to_reinforce, map().planet(id).current_num_ships());
      LOG << "sending reinforcements of " << map().planet(id).current_num_ships() << " ships to the planet "
          << best_planet_to_reinforce << "." << endl;
    } else LOG << "no potential target, no further action." << endl;
  } else LOG << "not enough ships, no further action." << endl;
}

// Compute the mean distance between each pair of nearest planets in number of turns
unsigned int SageBot::compute_planets_mean_distance_() const {
  unsigned long long int dist_sum = 0;

  for_each(map().planets_begin(), map().planets_end(), [this, &dist_sum](const Planet& planet) {
    unsigned int distance_to_nearest_planet = 1000;
    for_each(map().planets_begin(), map().planets_end(), [&planet,&distance_to_nearest_planet](const Planet& planet2) {
      if(planet2.id() != planet.id()) {
        const unsigned int dist = planet.compute_travel_distance(planet2);
        if(dist < distance_to_nearest_planet) distance_to_nearest_planet = dist;
      }
    });

    dist_sum += distance_to_nearest_planet;
  });

  return (unsigned int)(dist_sum/(unsigned long long int)map().num_planets());
}

// Precompute planets neighborhoods
void SageBot::compute_planets_neighborhoods_() {
  neighborhoods_.resize(map().num_planets());

  for(planet_id id = 1; id <= map().num_planets(); ++id) {
    for(planet_id id2 = 1; id2 <= map().num_planets(); ++id2) {
      if(id2 != id) {
        if(map().planet(id).compute_travel_distance(map().planet(id2)) <= neighborhood_radius_)
          neighborhoods_[id - 1].push_back(id2);
      }
    }
  }
}
