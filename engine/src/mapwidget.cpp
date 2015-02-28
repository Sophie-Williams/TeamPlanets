// mapwidget.cpp - MapWidget class implementation
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
#include <QtGui>
#include <algorithm>
#include "map.hpp"
#include "mapwidget.hpp"

using namespace std;
using namespace team_planets;
using namespace team_planets_engine;

MapWidget::MapWidget(QWidget* parent, Qt::WindowFlags flags):
  QWidget(parent, flags), map_mutex_(nullptr), map_(nullptr),
  background_color_(Qt::black), neutral_color_(Qt::green),
  border_margin_(2.5), planet_base_radius_(15.0), planet_radius_incr_per_ship_prod_(1.0) {
}

void MapWidget::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // Filling the background
  painter.fillRect(0, 0, width(), height(), QBrush(background_color_));

  // Drawing the planets
  map_mutex_->lock();
  compute_map_bounding_box_();

  for_each(map_->planets_begin(), map_->planets_end(), [this, &painter](const Planet& planet) {
    const QPointF planet_pos = compute_planet_location_in_widget_coordinates_(planet);
    const qreal planet_radius = planet_base_radius_ + (qreal)planet.ship_increase()*planet_radius_incr_per_ship_prod_;

    // Drawing planet
    painter.setPen(neutral_color_);
    painter.drawEllipse(planet_pos, planet_radius, planet_radius);

    // Draw the number of ships
    painter.drawText(QRectF(planet_pos.x() - planet_radius, planet_pos.y() - planet_radius,
                            2.0*planet_radius, 2.0*planet_radius),
                     Qt::AlignCenter,
                     tr("%1").arg(planet.current_num_ships()));
  });

  map_mutex_->unlock();
}

void MapWidget::compute_map_bounding_box_() {
  // Computing min-max coordinates of the planets
  float min_x = 0.0f, min_y = 0.0f, max_x = 0.0f, max_y = 0.0f;
  if(map_->num_planets() != 0) {
    min_x = map_->planets_begin()->location().x();
    min_y = map_->planets_begin()->location().y();
    max_x = map_->planets_begin()->location().x();
    max_y = map_->planets_begin()->location().y();
  }

  for_each(map_->planets_begin(), map_->planets_end(), [&min_x, &min_y, &max_x, &max_y](const Planet& planet) {
    if(min_x > planet.location().x()) min_x = planet.location().x();
    if(min_y > planet.location().y()) min_y = planet.location().y();
    if(max_x < planet.location().x()) max_x = planet.location().x();
    if(max_y < planet.location().y()) max_y = planet.location().y();
  });

  // Adding map margin
  min_x -= border_margin_; min_y -= border_margin_;
  max_x += border_margin_; max_y += border_margin_;

  map_bounding_box_ = QRectF(QPointF((qreal)min_x, (qreal)min_y), QPointF((qreal)max_x, (qreal)max_y));
}

QPointF MapWidget::compute_planet_location_in_widget_coordinates_(const Planet& planet) {
  const float x = (planet.location().x() - (float)map_bounding_box_.x())
      *((float)width()/(float)map_bounding_box_.width());
  const float y = (planet.location().y() - (float)map_bounding_box_.y())
      *((float)height()/(float)map_bounding_box_.height());
  return QPointF((qreal)x, (qreal)y);
}
