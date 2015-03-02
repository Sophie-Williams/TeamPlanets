// mainwindow.hpp - Application main window definition
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

#ifndef _TEAMPLANETS_TEAMPLANETSENGINE_MAINWINDOW_HPP_
#define _TEAMPLANETS_TEAMPLANETSENGINE_MAINWINDOW_HPP_

#include <QMainWindow>
#include <QString>
#include <QMutex>
#include "battlethread.hpp"
#include "map.hpp"
#include "ui_mainwindow.h"

namespace team_planets_engine {
  class MainWindow: public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(int argc, char* argv[], QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

  protected:
    virtual void showEvent(QShowEvent* event);
    virtual void closeEvent(QCloseEvent* event);

  private slots:
    void startBattleActionTriggered_();
    void quitActionTriggered_();

    void battle_thread_map_updated_();
    void battle_thread_error_occured(const QString& msg);

  private:
    Q_DISABLE_COPY(MainWindow)

    void buildInterface_();
    void connectSlots_();

    void parseCommandLine_();
    void startNewBattle_(QString map_file_name,
                         QString team1_bot_file_name, unsigned int team1_num_players,
                         QString team2_bot_file_name, unsigned int team2_num_players);
    void stopBattle_();

    void update_teams_tables_();

    // User interface
    Ui::MainWindow  ui_;

    // Application command line
    const int     argc_;
    const char**  argv_;

    // Application data
    BattleThread*     battle_thread_;

    QMutex            map_mutex_;
    team_planets::Map map_;
  };
}

#endif
