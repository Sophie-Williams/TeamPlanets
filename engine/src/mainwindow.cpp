// mainwindow.cpp - Application main window implementation
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

#include <QtWidgets>
#include <algorithm>
#include "startbattledialog.hpp"
#include "mainwindow.hpp"

using namespace team_planets_engine;

MainWindow::MainWindow(int argc, char* argv[], QWidget* parent, Qt::WindowFlags flags):
  QMainWindow(parent, flags), argc_(argc), argv_((const char**)argv), battle_thread_(nullptr) {
  buildInterface_();
  connectSlots_();
}

void MainWindow::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);

  static bool first_time_show_event = true;
  if(first_time_show_event) {
    first_time_show_event = false;
    parseCommandLine_();
  }
}

void MainWindow::closeEvent(QCloseEvent* event) {
  stopBattle_();
}

void MainWindow::startBattleActionTriggered_() {
  static StartBattleDialog* dialog  = nullptr;
  if(!dialog) dialog = new StartBattleDialog(this);

  if(dialog->exec())
    startNewBattle_(dialog->map_file_name(),
                    dialog->team1_bot_file_name(), dialog->team1_num_players(),
                    dialog->team2_bot_file_name(), dialog->team2_num_players());
}

void MainWindow::quitActionTriggered_() {
  stopBattle_();
  qApp->quit();
}

void MainWindow::battle_thread_map_updated_() {
  update_teams_tables_();
  ui_.battleMap->update();
}

void MainWindow::battle_thread_error_occured(const QString& msg) {
  QMessageBox::critical(this, tr("Battle error"), msg);
}

void MainWindow::buildInterface_() {
  // Creating the user interface
  ui_.setupUi(this);
  ui_.battleMap->set_map(map_mutex_, map_);

  // Moving main splitter handle to a more comfortable position
  ui_.mainSplitter->setStretchFactor(0, 2);
}

void MainWindow::connectSlots_() {
  connect(ui_.startBattleAction, &QAction::triggered, this, &MainWindow::startBattleActionTriggered_);
  connect(ui_.quitAction, &QAction::triggered, this, &MainWindow::quitActionTriggered_);
}

void MainWindow::parseCommandLine_() {
  if(argc_ > 5) {
    bool ok1 = false, ok2 = false;
    const unsigned int team1_num_players = QString(argv_[3]).toUInt(&ok1);
    const unsigned int team2_num_players = QString(argv_[5]).toUInt(&ok2);

    if(ok1 && ok2) startNewBattle_(argv_[1], argv_[2], team1_num_players, argv_[4], team2_num_players);
  }
}

void MainWindow::startNewBattle_(QString map_file_name,
                                 QString team1_bot_file_name, unsigned int team1_num_players,
                                 QString team2_bot_file_name, unsigned int team2_num_players) {
  // Get rid of the current battle thread
  if(battle_thread_) {
    battle_thread_->stop();
    battle_thread_->wait();
    battle_thread_->deleteLater();
    battle_thread_ = nullptr;
  }

  // Starting the new battle
  battle_thread_ = new BattleThread(map_file_name, team1_bot_file_name, team1_num_players,
                                    team2_bot_file_name, team2_num_players, map_mutex_, map_,
                                    this);
  connect(battle_thread_, &BattleThread::map_updated, this, &MainWindow::battle_thread_map_updated_);
  connect(battle_thread_, &BattleThread::error_occured, this, &MainWindow::battle_thread_error_occured);

  ui_.battleMap->set_battle_thread(battle_thread_);
  battle_thread_->start();
}

void MainWindow::stopBattle_() {
  if(battle_thread_) {
    battle_thread_->stop();
    battle_thread_->wait();
  }
}

void MainWindow::update_teams_tables_() {
  QStringList header;
  header << "ID" << "C" << "S" << "Planets" << "Ships" << "Ping (ms)";

  if(battle_thread_) {
    // Updating the first team table
    ui_.team1Table->clear();
    ui_.team1Table->setColumnCount(6);
    ui_.team1Table->setRowCount(battle_thread_->team1_num_players());
    ui_.team1Table->setHorizontalHeaderLabels(header);
    ui_.team1Table->verticalHeader()->hide();

    // Updating the second team table
    ui_.team2Table->clear();
    ui_.team2Table->setColumnCount(6);
    ui_.team2Table->setRowCount(battle_thread_->team2_num_players());
    ui_.team2Table->setHorizontalHeaderLabels(header);
    ui_.team2Table->verticalHeader()->hide();

    // Filling in the content
    int team1_row = 0, team2_row = 0;

    battle_thread_->lock_players();
    std::for_each(battle_thread_->players_begin(), battle_thread_->players_end(),
                  [this,&team1_row,&team2_row](const Player& player) {
      QTableWidgetItem* id_item = new QTableWidgetItem(QString("%1").arg(player.id()));
      QTableWidgetItem* color_item = new QTableWidgetItem;
      color_item->setBackgroundColor(player.color());

      QTableWidgetItem* status_item = nullptr;
      switch(player.status()) {
      case Player::Alive: status_item =  new QTableWidgetItem(tr("A")); break;
      case Player::Dead: status_item =  new QTableWidgetItem(tr("D")); break;
      case Player::Failed: status_item =  new QTableWidgetItem(tr("F")); break;
      }

      QTableWidgetItem* planets_item = new QTableWidgetItem(QString("%1").arg(player.num_planets()));
      QTableWidgetItem* ships_item = new QTableWidgetItem(QString("%1").arg(player.num_ships()));
      QTableWidgetItem* ping_item = new QTableWidgetItem(QString("%1").arg(player.ping()));

      if(player.team() == 1) {
        ui_.team1Table->setItem(team1_row, 0, id_item);
        ui_.team1Table->setItem(team1_row, 1, color_item);
        ui_.team1Table->setItem(team1_row, 2, status_item);
        ui_.team1Table->setItem(team1_row, 3, planets_item);
        ui_.team1Table->setItem(team1_row, 4, ships_item);
        ui_.team1Table->setItem(team1_row, 5, ping_item);
        ++team1_row;
      } else {
        ui_.team2Table->setItem(team2_row, 0, id_item);
        ui_.team2Table->setItem(team2_row, 1, color_item);
        ui_.team2Table->setItem(team2_row, 2, status_item);
        ui_.team2Table->setItem(team2_row, 3, planets_item);
        ui_.team2Table->setItem(team2_row, 4, ships_item);
        ui_.team2Table->setItem(team2_row, 5, ping_item);
        ++team2_row;
      }
    });
    battle_thread_->unlock_players();

    ui_.team1Table->resizeColumnsToContents();
    ui_.team2Table->resizeColumnsToContents();
  }
}
