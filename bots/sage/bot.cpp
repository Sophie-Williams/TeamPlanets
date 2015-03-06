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
#include "log.hpp"
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

void Bot::init_() {
}

void Bot::perform_turn_() {
}

void Bot::begin_turn_() {
  map_.bot_begin_turn();

  // Perform initialization
  if(!initialized_) initialize_bot_();

  // Update the list of team mates
  if(!team_.is_complete()) map_.set_message(team_.process_message(map_.myself(), map_.message()));

  LOG << "Begin turn " << current_turn_ << "..." << endl;
  LOG << "team = " << team_ << endl;
}

void Bot::end_turn_() {
  map_.bot_end_turn();
  ++current_turn_;
  LOG << endl;
}

void Bot::initialize_bot_() {
  Log::Instance().init_log(map_.myself());
  init_();
  initialized_ = true;
}
