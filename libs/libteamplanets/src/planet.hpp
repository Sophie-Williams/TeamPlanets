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
// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

#ifndef _TEAMPLANETS_LIBTEAMPLANETS_PLANET_HPP_
#define _TEAMPLANETS_LIBTEAMPLANETS_PLANET_HPP_

#include <cassert>
#include <cmath>
#include <iostream>
#include "basic_types.hpp"
#include "coordinates.hpp"

namespace team_planets {
  class Planet {
  public:
    Planet():
      id_(0), ship_increase_(0), current_owner_(neutral_player), current_num_ships_(0) {}
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
    void remove_ships(unsigned int num_ships) {
      assert(num_ships <= current_num_ships_);
      current_num_ships_ -= num_ships;
    }

  private:
    template<typename charT, typename traits>
    friend std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& out, const Planet& P);
    template<typename charT, typename traits>
    friend std::basic_istream<charT,traits>& operator>>(std::basic_istream<charT,traits>& in, Planet& P);

    // Constant planet data
    planet_id     id_;            // The planet ID
    Coordinates   location_;      // The planet location
    unsigned int  ship_increase_; // Number of ships added per turn
    
    // Variable planet data
    player_id     current_owner_;           // The current owner of the planet
    unsigned int  current_num_ships_;       // Number of ships currently on this planet         
  };

  // Input/output operators
  template<typename charT, typename traits>
  std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& out, const Planet& P) {
    out << "P " << P.id_ << ' ' << P.location_ << ' ';
    out << P.ship_increase_ << ' ' << P.current_owner_ << ' ' << P.current_num_ships_;
    return out;
  }

  template<typename charT, typename traits>
  std::basic_istream<charT,traits>& operator>>(std::basic_istream<charT,traits>& in, Planet& P) {
    in >> P.id_ >> P.location_ >> P.ship_increase_ >> P.current_owner_ >> P.current_num_ships_;
    return in;
  }
}

#endif
