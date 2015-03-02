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
#include <algorithm>
#include "map.hpp"
#include "battlethread.hpp"

using namespace std;
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

    // Creating the players
    create_players_();
    update_players_();

    // Battle main loop
    bool stop_requested = false;
    while(!stop_requested) {
      // Manually process pending events
      QCoreApplication::sendPostedEvents();
      QCoreApplication::processEvents();

      // Check if the thread must stop
      stop_mutex_.lock();
      stop_requested = stop_;
      stop_mutex_.unlock();
    }

    // Clean up
    destroy_players_();
  } catch(const std::exception& e) {
    emit error_occured(QString(e.what()));
    qDebug() << "ERROR:" << e.what();
    qDebug() << "The battle was aborted due to an error!";
  }

  qDebug() << "The battle is over.";
}

void BattleThread::bot_error(QProcess::ProcessError error) {
  if(error == QProcess::Crashed) {
    // One of the bots have crashed
    players_mutex_.lock();
    for(player_id id = 1; id <= players_.size(); ++id) {
      if(players_[id - 1].status() == Player::Alive && bots_[id - 1]->state() == QProcess::NotRunning)
        players_[id - 1].set_status(Player::Failed);
    }
    players_mutex_.unlock();
  }
}

void BattleThread::create_players_() {
  players_mutex_.lock();
  players_.clear();

  qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");

  // Creating the first team
  int cur_color = 255;
  int color_step = 200/team1_num_players_;
  for(unsigned int i = 0; i < team1_num_players_; ++i) {
    players_.push_back(Player(players_.size() + 1, 1, QColor(cur_color, 0, 0)));
    cur_color -= color_step;

    qDebug() << "Starting bot " << team1_bot_file_name_ << " for player " << players_.back().id();
    QProcess* player_process = new QProcess;
    connect(player_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(bot_error(QProcess::ProcessError)));
    player_process->start(team1_bot_file_name_);
    if(!player_process->waitForStarted(10000)) players_.back().set_status(Player::Failed);
    bots_.push_back(player_process);
  }

  // Creating the second team
  cur_color = 255;
  color_step = 200/team2_num_players_;
  for(unsigned int i = 0; i < team2_num_players_; ++i) {
    players_.push_back(Player(players_.size() + 1, 2, QColor(0, 0, cur_color)));
    cur_color -= color_step;

    qDebug() << "Starting bot " << team2_bot_file_name_ << " for player " << players_.back().id();
    QProcess* player_process = new QProcess;
    connect(player_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(bot_error(QProcess::ProcessError)));
    player_process->start(team2_bot_file_name_);
    if(!player_process->waitForStarted(10000)) players_.back().set_status(Player::Failed);
    bots_.push_back(player_process);
  }

  players_mutex_.unlock();
}

void BattleThread::update_players_() {
  players_mutex_.lock();

  // Reset the player statistics
  for(Player& player : players_) {
    player.set_num_planets(0);
    player.set_num_ships(0);
  }

  // Updating
  map_mutex_.lock();
  for_each(map_.planets_begin(), map_.planets_end(), [this](const Planet& planet) {
    if(planet.current_owner() != neutral_player) {
      Player& owner = players_[planet.current_owner() - 1];
      owner.set_num_planets(owner.num_planets() + 1);
      owner.set_num_ships(owner.num_ships() + planet.current_num_ships());
    }
  });

  for_each(map_.fleets_begin(), map_.fleets_end(), [this](const Fleet& fleet) {
    Player& owner = players_[fleet.player() - 1];
    owner.set_num_ships(owner.num_ships() + fleet.num_ships());
  });
  map_mutex_.unlock();

  players_mutex_.unlock();
}

void BattleThread::destroy_players_() {
  qDebug() << "Terminating bots...";
  for(QProcess* bot:bots_) {
    bot->terminate();
    if(!bot->waitForFinished(1000)) bot->kill();
    delete bot;
  }

  bots_.clear();
}
