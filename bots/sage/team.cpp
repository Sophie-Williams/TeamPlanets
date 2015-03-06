// team.cpp - Team class implementation
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
#include "planet.hpp"
#include "team.hpp"

using namespace std;
using namespace team_planets;
using namespace sage;

Team::Team():
  teammate_id_to_send_(0), team_is_complete_(false) {
}

// Planet ownership tests
bool Team::is_owned_by_my_team(const Planet& planet) const {
  auto it = find(team_.begin(), team_.end(), planet.current_owner());
  return it != end(team_);
}

bool Team::is_owned_by_enemy_team(const Planet& planet) const {
  return planet.current_owner() != neutral_player && !is_owned_by_my_team(planet);
}

uint32_t Team::process_message(player_id myself, uint32_t msg) {
  if(team_.empty()) team_.push_back(myself);

  // Processing input message
  if(msg != 0) {
    if(msg != (uint32_t)myself) team_.push_back((player_id)msg);
    else team_is_complete_ = true;
  }

  // Generating output message
  uint32_t ret = 0;
  if(teammate_id_to_send_ < team_.size()) {
    ret = (uint32_t)team_[teammate_id_to_send_];
    ++teammate_id_to_send_;
  }

  return ret;
}
