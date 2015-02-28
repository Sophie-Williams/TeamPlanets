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
#include "startbattledialog.hpp"
#include "mainwindow.hpp"

using namespace team_planets_engine;

MainWindow::MainWindow(int argc, char* argv[], QWidget* parent, Qt::WindowFlags flags):
  QMainWindow(parent, flags), argc_(argc), argv_((const char**)argv) {
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

void MainWindow::startBattleActionTriggered_() {
  static StartBattleDialog* dialog  = nullptr;
  if(!dialog) dialog = new StartBattleDialog(this);

  if(dialog->exec()) startNewBattle_(dialog->map_file_name());
}

void MainWindow::quitActionTriggered_() {
  qApp->quit();
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
  if(argc_ > 1) {
    startNewBattle_(argv_[1]);
  }
}

void MainWindow::startNewBattle_(QString map_file_name) {
  qDebug() << "Starting new battle...";

  try {
    map_.load_google_ai_challenge_map(map_file_name.toStdString());
    qDebug() << "Loaded map from" << map_file_name << ":" << map_.num_planets() << " planets.";
  } catch(const std::exception& e) {
    QMessageBox::critical(this, tr("Unable to start a battle"), QString(e.what()));
    qDebug() << "ERROR:" << e.what();
    qDebug() << "Starting of a new battle was aborted due to an error!";
  }
}
