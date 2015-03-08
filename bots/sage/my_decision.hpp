// my_decision.hpp - MyDecision class definition
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

#ifndef _TEAMPLANETS_SAGE_MY_DECISION_HPP_
#define _TEAMPLANETS_SAGE_MY_DECISION_HPP_

#include <vector>
#include "decision.hpp"

namespace sage {
  class MyDecision: public Decision {
  public:
    MyDecision(const SageBot& bot, const team_planets::Map& map);
    virtual ~MyDecision();

    virtual decisions_list generate_decisions();

  private:
    // User defined bot parameters
    const unsigned int  num_ships_per_reinforcement_;

    // List of potential targets
    struct Target_ {
      team_planets::planet_id id;
      float                   score;
    };
    typedef std::vector<Target_>          targets_list_;

    targets_list_ potential_targets_;
    void compute_list_of_potential_targets_();

    // Decision recursive state
    typedef std::vector<unsigned int> remaining_ships_list_;
    struct DecisionState_ {
      orders_list             current_decision;
      targets_list_           potential_targets;
      remaining_ships_list_   remaining_ships;
    };

    decisions_list recursively_generate_decisions_() const;
    DecisionState_ generate_initial_decision_state_() const;
    orders_list generate_toplevel_orders_for_a_given_state_(const DecisionState_& state) const;

    orders_list generate_backline_planets_orders_() const;
  };
}

#endif
