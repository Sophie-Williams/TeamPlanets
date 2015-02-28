// startbattledialog.hpp - StartBattleDialog class definition
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

#ifndef _TEAMPLANETS_TEAMPLANETSENGINE_STARTBATTLEDIALOG_HPP_
#define _TEAMPLANETS_TEAMPLANETSENGINE_STARTBATTLEDIALOG_HPP_

#include <QDialog>
#include <QString>
#include "ui_startbattledialog.h"

namespace team_planets_engine {
  class StartBattleDialog: public QDialog {
    Q_OBJECT

  public:
    explicit StartBattleDialog(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

    QString map_file_name() const { return ui_.mapLineEdit->text(); }

  private slots:
    void mapButtonClicked_();
    void team1ButtonClicked_();
    void team2ButtonClicked_();

    void checkUserInput_();

  private:
    Q_DISABLE_COPY(StartBattleDialog)

    void buildInterface_();
    void connectSlots_();

    Ui::StartBattleDialog ui_;
  };
}

#endif
