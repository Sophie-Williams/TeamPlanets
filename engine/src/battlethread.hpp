// battlethread.hpp - BattleThread class definition
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

#ifndef _TEAMPLANETS_TEAMPLANETSENGINE_BATTLETHREAD_HPP_
#define _TEAMPLANETS_TEAMPLANETSENGINE_BATTLETHREAD_HPP_

#include <QThread>
#include <QMutex>
#include <QString>
#include <cassert>
#include <vector>
#include "player.hpp"

namespace team_planets { class Map; }

namespace team_planets_engine {
  class BattleThread: public QThread {
    Q_OBJECT

  private:
    typedef std::vector<Player> players_list;

  public:
    typedef players_list::const_iterator  player_const_iterator;

    BattleThread(const QString& map_file_name, const QString& team1_bot_file_name, unsigned int team1_num_players,
                 const QString& team2_bot_file_name, unsigned int team2_num_players,
                 QMutex& map_mutex, team_planets::Map& map, QObject* parent = nullptr);

    void stop() { stop_mutex_.lock(); stop_ = true; stop_mutex_.unlock(); }

    // Players list accessors
    void lock_players() { players_mutex_.lock(); }
    void unlock_players() { players_mutex_.unlock(); }

    std::size_t num_players() const { return players_.size(); }
    const Player& player(team_planets::player_id id) const { assert(id != 0); return players_[id - 1]; }
    player_const_iterator players_begin() const { return players_.begin(); }
    player_const_iterator players_end() const { return players_.end(); }

  signals:
    void map_updated();
    void error_occured(const QString& msg);

  protected:
    virtual void run();

  private:
    Q_DISABLE_COPY(BattleThread)

    void create_players_();
    void update_players_();

    // Thread management data
    QMutex  stop_mutex_;
    bool    stop_;

    // Battle configuration
    const QString map_file_name_;
    const QString team1_bot_file_name_;
    const unsigned int team1_num_players_;
    const QString team2_bot_file_name_;
    const unsigned int team2_num_players_;

    // Battle map references
    QMutex&             map_mutex_;
    team_planets::Map&  map_;

    // Players and the associated bots
    QMutex        players_mutex_;
    players_list  players_;
  };
}

#endif
