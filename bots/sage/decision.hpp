// decision.hpp - Decision class definition
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

#ifndef _TEAMPLANETS_SAGE_DECISION_HPP_
#define _TEAMPLANETS_SAGE_DECISION_HPP_

#include <vector>
#include "sagebot.hpp"

namespace sage {
  class Decision {
  public:
    typedef std::vector<team_planets::planet_id>  planets_list;
    typedef std::vector<team_planets::Fleet>      orders_list;
    typedef std::vector<orders_list>              decisions_list;

    Decision(const SageBot& bot, const team_planets::Map& map);
    virtual ~Decision();

    virtual decisions_list generate_decisions();

  protected:
    // Bot and map accessors
    const SageBot& bot() const { return bot_; }
    const team_planets::Map& map() const { return map_; }

    // Frontline and backline lists accessors
    planets_list& frontline_planets() { return frontline_planets_; }
    const planets_list& frontline_planets() const { return frontline_planets_; }
    planets_list& backline_planets() { return backline_planets_; }
    const planets_list& backline_planets() const { return backline_planets_; }

    // Some common tools to make a decisions
    void perform_backline_frontline_classification_();
    unsigned int num_ships_to_take_a_planet_(team_planets::planet_id src, team_planets::planet_id dst) const;

    // Recursive orders generation common for my and enemy decision
    typedef std::vector<unsigned int> remaining_ships_list_;
    struct DecisionState_ {
      orders_list             current_decision;
      planets_list            potential_targets;
      remaining_ships_list_   remaining_ships;
    };

    decisions_list recursively_generate_decisions_(const planets_list& sources, const planets_list& destinations) const;
    DecisionState_ generate_initial_decision_state_(const planets_list& sources,
                                                    const planets_list& destinations) const;
    orders_list generate_toplevel_orders_for_a_given_state_(const planets_list& sources,
                                                            const DecisionState_& state) const;

  private:
    bool is_frontline_(team_planets::planet_id id) const;

    const SageBot&            bot_;
    const team_planets::Map&  map_;

    planets_list  frontline_planets_;
    planets_list  backline_planets_;
  };
}

#endif
