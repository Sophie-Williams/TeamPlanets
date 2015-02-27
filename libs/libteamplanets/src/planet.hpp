// planet.hpp - Planet class definition
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
// ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

#ifndef _TEAMPLANETS_LIBTEAMPLANETS_PLANET_HPP_
#define _TEAMPLANETS_LIBTEAMPLANETS_PLANET_HPP_

#include <cmath>
#include "basic_types.hpp"
#include "coordinates.hpp"

namespace team_planets {
  class Planet {
  public:
    Planet(planet_id id, const Coordinates& location, unsigned int ship_increase,
           player_id current_owner = neutral_player, unsigned int current_num_ships = 0):
      id_(id), location_(location), ship_increase_(ship_increase), current_owner_(current_owner),
      current_num_ships_(current_num_ships) {}
    
    // Constant planet data accessors
    planet_id id() const { return id_; }
    const Coordinates& location() const { return location_; }
    unsigned int ship_increase() const { return ship_increase_; }
    
    // Variable planet data accessors
    player_id current_owner() const { return current_owner_; }
    void set_current_owner(player_id new_owner) { current_owner_ = new_owner; }
    unsigned int current_num_ships() const { return current_num_ships_; }
    void set_current_num_ships(unsigned int new_num_ships) { current_num_ships_ = new_num_ships; }
    
    // Various game mechanics functions
    void produce_new_ships() { current_num_ships_ += ship_increase_; }
    
    unsigned int compute_travel_distance(const Planet& other_planet) {
      return (unsigned int)std::trunc(location_.euclidian_distance(other_planet.location_));
    }
    
  private:
    // Constant planet data
    const planet_id     id_;            // The planet ID
    const Coordinates   location_;      // The planet location
    const unsigned int  ship_increase_; // Number of ships added per turn
    
    // Variable planet data
    player_id     current_owner_;           // The current owner of the planet
    unsigned int  current_num_ships_;       // Number of ships currently on this planet         
  };
}

#endif
