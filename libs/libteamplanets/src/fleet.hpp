// fleet.hpp - Fleet class definition
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

#ifndef _TEAMPLANETS_LIBTEAMPLANETS_FLEET_HPP_
#define _TEAMPLANETS_LIBTEAMPLANETS_FLEET_HPP_

#include <cassert>
#include "basic_types.hpp"

namespace team_planets {
  class Fleet {
  public:
    Fleet():
      player_(neutral_player), source_(0), destination_(0),
      num_ships_(0), remaining_turns_(0) {}
    Fleet(player_id player, planet_id source, planet_id destination,
          unsigned int num_ships, unsigned int remaining_turns):
      player_(player), source_(source), destination_(destination),
      num_ships_(num_ships), remaining_turns_(remaining_turns) {}

    // Constant fleet data accessors
    player_id player() const { return player_; }
    planet_id source() const { return source_; }
    planet_id destination() const { return destination_; }
    unsigned int num_ships() const { return num_ships_; }

    // Variable fleet data accessors
    unsigned int remaining_turns() const { return remaining_turns_; }

    // Various game mechanics functions
    void advance() { assert(remaining_turns_ != 0); --remaining_turns_; }

  private:
    template<typename charT, typename traits>
    friend std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& out, const Fleet& F);
    template<typename charT, typename traits>
    friend std::basic_istream<charT,traits>& operator>>(std::basic_istream<charT,traits>& in, Fleet& F);

    // Constant fleet data
    player_id     player_;
    planet_id     source_;
    planet_id     destination_;
    unsigned int  num_ships_;

    // Variable fleet data
    unsigned int        remaining_turns_;
  };

  // Input/output operators
  template<typename charT, typename traits>
  std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& out, const Fleet& F) {
    out << "F " << F.source_ << " " << F.destination_ << " " << F.num_ships_;
    return out;
  }

  template<typename charT, typename traits>
  std::basic_istream<charT,traits>& operator>>(std::basic_istream<charT,traits>& in, Fleet& F) {
    in >> F.source_ >> F.destination_ >> F.num_ships_;
    return in;
  }
}

#endif
