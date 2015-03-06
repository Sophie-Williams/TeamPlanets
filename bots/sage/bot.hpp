// bot.hpp - Bot class definition
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

#ifndef _TEAMPLANETS_SAGE_BOT_HPP_
#define _TEAMPLANETS_SAGE_BOT_HPP_

#include <vector>
#include "map.hpp"
#include "team.hpp"
#include "utils.hpp"

namespace sage {
  class Bot {
  public:
    DISABLE_COPY(Bot)

    Bot(): initialized_(false), current_turn_(1) {}
    virtual ~Bot() {}

    int run();

  protected:
    // Various general info
    unsigned int current_turn() const { return current_turn_; }

    // Access to the map
    team_planets::Map& map() { return map_; }
    const team_planets::Map& map() const { return map_; }

    // Planet ownership checks
    bool team_is_complete_() const { return team_.is_complete(); }
    bool is_owned_by_me_(const team_planets::Planet& planet) const { return planet.current_owner() == map_.myself(); }
    bool is_neutral_(const team_planets::Planet& planet) const { return planet.current_owner() == neutral_player; }
    bool is_owned_by_my_team_(const team_planets::Planet& planet) const { return team_.is_owned_by_my_team(planet); }
    bool is_owned_by_enemy_team_(const team_planets::Planet& planet) const {
      return team_.is_owned_by_enemy_team(planet);
    }

    // Function to overload
    virtual void init_();
    virtual void perform_turn_();

  private:
    void begin_turn_();
    void end_turn_();

    void initialize_bot_();

    bool                                  initialized_;
    unsigned int                          current_turn_;

    team_planets::Map                     map_;
    Team                                  team_;
  };
}

#endif
