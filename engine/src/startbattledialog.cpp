// startbattledialog.cpp - StartBattleDialog class implementation
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

using namespace team_planets_engine;

StartBattleDialog::StartBattleDialog(QWidget* parent, Qt::WindowFlags flags):
  QDialog(parent, flags) {
  buildInterface_();
  connectSlots_();
}

void StartBattleDialog::mapButtonClicked_() {
  QString file_name = QFileDialog::getOpenFileName(this, tr("Open map..."), ui_.mapLineEdit->text(),
                                                   tr("Google AI challenge map files (*.txt);;All files (*)"));

  if(!file_name.isEmpty()) ui_.mapLineEdit->setText(file_name);
}

void StartBattleDialog::team1ButtonClicked_() {
  QString file_name = getBotFileName_(ui_.team1LineEdit->text());
  if(!file_name.isEmpty()) ui_.team1LineEdit->setText(file_name);
}

void StartBattleDialog::team2ButtonClicked_() {
  QString file_name = getBotFileName_(ui_.team2LineEdit->text());
  if(!file_name.isEmpty()) ui_.team2LineEdit->setText(file_name);
}

void StartBattleDialog::checkUserInput_() {
  ui_.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!ui_.mapLineEdit->text().isEmpty()
                                                          && !ui_.team1LineEdit->text().isEmpty()
                                                          && !ui_.team2LineEdit->text().isEmpty());
}

void StartBattleDialog::buildInterface_() {
  ui_.setupUi(this);
  checkUserInput_();
}

void StartBattleDialog::connectSlots_() {
  connect(ui_.mapButton, &QPushButton::clicked, this, &StartBattleDialog::mapButtonClicked_);
  connect(ui_.team1Button, &QPushButton::clicked, this, &StartBattleDialog::team1ButtonClicked_);
  connect(ui_.team2Button, &QPushButton::clicked, this, &StartBattleDialog::team2ButtonClicked_);

  connect(ui_.mapLineEdit, &QLineEdit::textChanged, this, &StartBattleDialog::checkUserInput_);
  connect(ui_.team1LineEdit, &QLineEdit::textChanged, this, &StartBattleDialog::checkUserInput_);
  connect(ui_.team2LineEdit, &QLineEdit::textChanged, this, &StartBattleDialog::checkUserInput_);
}

QString StartBattleDialog::getBotFileName_(QString current_file_name) {
  QString file_name = QFileDialog::getOpenFileName(this, tr("Open bot executable..."), current_file_name,
                                                   tr("All files (*)"));
  return file_name;
}
