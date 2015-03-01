// battlethread.cpp - BattleThread class implementation
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

#include <QtCore>
#include "map.hpp"
#include "battlethread.hpp"

using namespace team_planets;
using namespace team_planets_engine;

BattleThread::BattleThread(const QString& map_file_name,
                           const QString& team1_bot_file_name, unsigned int team1_num_players,
                           const QString& team2_bot_file_name, unsigned int team2_num_players,
                           QMutex& map_mutex, team_planets::Map& map, QObject* parent):
  QThread(parent), stop_(false), map_file_name_(map_file_name), team1_bot_file_name_(team1_bot_file_name),
  team1_num_players_(team1_num_players), team2_bot_file_name_(team2_bot_file_name),
  team2_num_players_(team2_num_players), map_mutex_(map_mutex), map_(map) {
}

void BattleThread::run() {
  qDebug() << "Starting new battle...";

  try {
    // Loading the battle map
    map_mutex_.lock();
    map_.reset();
    map_.load(map_file_name_.toStdString());
    qDebug() << "Loaded map from" << map_file_name_ << ":" << map_.num_planets() << " planets.";
    map_mutex_.unlock();
    emit map_updated();

    qDebug() << "Team 1: bot =" << team1_bot_file_name_ << " players = " << team1_num_players_;
    qDebug() << "Team 2: bot =" << team2_bot_file_name_ << " players = " << team2_num_players_;

    // Battle main loop
    bool stop_requested = false;
    while(!stop_requested) {
      stop_mutex_.lock();
      stop_requested = stop_;
      stop_mutex_.unlock();
    }
  } catch(const std::exception& e) {
    emit error_occured(QString(e.what()));
    qDebug() << "ERROR:" << e.what();
    qDebug() << "The battle was aborted due to an error!";
  }

  qDebug() << "The battle is over.";
}
