// map.hpp - Map class definition
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

#ifndef _TEAMPLANETS_LIBTEAMPLANETS_MAP_HPP_
#define _TEAMPLANETS_LIBTEAMPLANETS_MAP_HPP_

#include <cassert>
#include <vector>
#include <string>
#include "planet.hpp"

namespace team_planets {
  class Map {
  private:
    typedef std::vector<Planet> planet_list;

  public:
    typedef planet_list::iterator       planet_iterator;
    typedef planet_list::const_iterator planet_const_iterator;

    // Map loading functions
    void load_google_ai_challenge_map(const std::string& file_name);

    // Planets accessors
    std::size_t num_planets() const { return planets_.size(); }
    Planet& planet(planet_id id) {
      assert(id != 0);
      assert(id - 1 < planets_.size());

      return planets_[id - 1];
    }
    const Planet& planet(planet_id id) const {
      assert(id != 0);
      assert(id - 1 < planets_.size());

      return planets_[id - 1];
    }

    planet_iterator planets_begin() { return planets_.begin(); }
    planet_const_iterator planets_begin() const { return planets_.begin(); }
    planet_iterator planets_end() { return planets_.end(); }
    planet_const_iterator planets_end() const { return planets_.end(); }

  private:
    planet_list planets_;
  };
}

#endif
