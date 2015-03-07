// sagebot.hpp - SageBot class definition
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

#ifndef _TEAMPLANETS_SAGE_SAGE_HPP_
#define _TEAMPLANETS_SAGE_SAGE_HPP_

#include <vector>
#include "bot.hpp"
#include "utils.hpp"

namespace sage {
  class SageBot: public Bot {
  private:
    typedef std::vector<team_planets::planet_id>  neighbors_list;
    typedef std::vector<neighbors_list>           neighborhoods_list;

  public:
    DISABLE_COPY(SageBot)

    SageBot(): num_ships_per_reinforcement_(10), planets_mean_distance_(0),
      neighborhood_radius_multiplier_(1), neighborhood_radius_(0) {}
    virtual ~SageBot() {}

  protected:
    virtual void init_();
    virtual void perform_turn_();

    neighbors_list& neighbors_(team_planets::planet_id planet) { return neighborhoods_[planet - 1]; }
    const neighbors_list& neighbors_(team_planets::planet_id planet) const { return neighborhoods_[planet - 1]; }

    bool is_frontline_(team_planets::planet_id id) const;
    unsigned int num_ships_to_take_a_planet_(team_planets::planet_id src, team_planets::planet_id dst) const;

    void take_attack_decisions_();
    void process_backline_planet_(team_planets::planet_id id);

  private:
    unsigned int compute_planets_mean_distance_() const;
    void compute_planets_neighborhoods_();

    // User defined bot parameters
    const unsigned int  num_ships_per_reinforcement_;

    // Precomputed map parameters
    unsigned int  planets_mean_distance_;
    unsigned int  neighborhood_radius_multiplier_;
    unsigned int  neighborhood_radius_;

    // Precomputed planets neighborhoods
    neighborhoods_list                            neighborhoods_;

    // Per turn data structures
    std::vector<team_planets::planet_id>  frontline_planets_;
    std::vector<team_planets::planet_id>  backline_planets_;
  };
}

#endif
