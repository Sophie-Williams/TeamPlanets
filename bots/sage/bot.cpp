// bot.cpp - Bot class implementation
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

#include <cstdlib>
#include <algorithm>
#include "bot.hpp"

using namespace std;
using namespace team_planets;
using namespace sage;

int Bot::run() {
  while(true) {
    begin_turn_();
    perform_turn_();
    end_turn_();
  }

  return EXIT_FAILURE;
}

bool Bot::is_owned_by_my_team_(const Planet& planet) const {
  auto it = find(my_team_.begin(), my_team_.end(), planet.current_owner());
  return it != end(my_team_);
}

void Bot::perform_turn_() {
}

void Bot::begin_turn_() {
  map_.bot_begin_turn();

  // Update the list of team mates
  if(my_team_.empty()) my_team_.push_back(map_.myself());
  if(map_.message() != 0 && map_.message() != (uint32_t)map_.myself())
    my_team_.push_back((player_id)map_.message());
}

void Bot::end_turn_() {
  // Generating the message to the team
  if(teammate_id_to_send_ < my_team_.size()) {
    map_.set_message((uint32_t)my_team_[teammate_id_to_send_]);
    ++teammate_id_to_send_;
  } else map_.set_message(0);

  map_.bot_end_turn();
}
