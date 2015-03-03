// player.hpp - Player class definition
// TeamPlanetsEngine - TeamPlanets game engine
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

#ifndef _TEAMPLANETS_TEAMPLANETSENGINE_PLAYER_HPP_
#define _TEAMPLANETS_TEAMPLANETSENGINE_PLAYER_HPP_

#include <cstdint>
#include <qcolor.h>
#include "basic_types.hpp"

namespace team_planets_engine {
  class Player {
  public:
    enum Status { Alive, Dead, Failed };

    Player(team_planets::player_id id, unsigned int team, QColor color);

    team_planets::player_id id() const { return id_; }
    unsigned int team() const { return team_; }
    QColor color() const { return color_; }

    Status status() const { return status_; }
    void set_status(Status status) { status_ = status; }
    unsigned int num_planets() const { return num_planets_; }
    void set_num_planets(unsigned int num_planets) { num_planets_ = num_planets; }
    unsigned int num_ships() const { return num_ships_; }
    void set_num_ships(unsigned int num_ships) { num_ships_ = num_ships; }

    unsigned int ping() const { return ping_; }
    void set_ping(unsigned int ping) { ping_ = ping; }

    uint32_t message() const { return message_; }
    void set_message(uint32_t message) { message_ = message; }

  private:
    // Player properties
    team_planets::player_id id_;
    unsigned int            team_;
    QColor                  color_;

    // Player statistics
    Status                  status_;
    unsigned int            num_planets_;
    unsigned int            num_ships_;

    unsigned int            ping_;

    // Player team message
    uint32_t                message_;
  };
}

#endif
