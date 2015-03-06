// team.hpp - Team class definition
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

#ifndef _TEAMPLANETS_SAGE_TEAM_HPP_
#define _TEAMPLANETS_SAGE_TEAM_HPP_

#include <cstdint>
#include <vector>
#include <ostream>
#include "basic_types.hpp"

namespace team_planets { class Planet; }

namespace sage {
  class Team {
  public:
    Team();

    // Planet ownership tests
    bool is_owned_by_my_team(const team_planets::Planet& planet) const;
    bool is_owned_by_enemy_team(const team_planets::Planet& planet) const;

    // Team management
    bool is_complete() const { return team_is_complete_; }
    uint32_t process_message(team_planets::player_id myself, uint32_t msg);

  private:
    template<typename charT, typename traits>
    friend std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& out, const Team& team);

    std::vector<team_planets::player_id>  team_;
    std::size_t                           teammate_id_to_send_;
    bool                                  team_is_complete_;
  };

  template<typename charT, typename traits>
  std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& out, const Team& team) {
    for(team_planets::player_id id:team.team_) out << id << " ";
    out << ", complete = ";
    if(team.team_is_complete_) out << "true";
    else out << "false";

    return out;
  }
}

#endif
