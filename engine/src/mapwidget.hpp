// mapwidget.hpp - MapWidget definition
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

#ifndef _TEAMPLANETS_TEAMPLANETSENGINE_MAPWIDGET_HPP_
#define _TEAMPLANETS_TEAMPLANETSENGINE_MAPWIDGET_HPP_

#include <QWidget>
#include <QColor>

class QMutex;
namespace team_planets { class Map; class Planet; class Fleet; }

namespace team_planets_engine {
  class BattleThread;

  class MapWidget: public QWidget {
    Q_OBJECT
    Q_PROPERTY(QColor background_color READ background_color WRITE set_background_color)
    Q_PROPERTY(QColor neutral_color READ neutral_color WRITE set_neutral_color)
    Q_PROPERTY(qreal border_margin READ border_margin WRITE set_border_margin)
    Q_PROPERTY(qreal planet_base_radius READ planet_base_radius WRITE set_planet_base_radius)
    Q_PROPERTY(qreal planet_radius_incr_per_ship_prod
               READ planet_radius_incr_per_ship_prod
               WRITE set_planet_radius_incr_per_ship_prod)
    Q_PROPERTY(qreal fleet_size READ fleet_size WRITE set_fleet_size)

  public:
    explicit MapWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

    void set_map(QMutex& map_mutex, team_planets::Map& map) {
      map_mutex_ = &map_mutex;
      map_ = &map;
    }
    void set_battle_thread(BattleThread* thread) { battle_thread_ = thread; }

    // Different map colors accessors
    QColor background_color() const { return background_color_; }
    void set_background_color(QColor background_color) { background_color_ = background_color; }
    QColor neutral_color() const { return neutral_color_; }
    void set_neutral_color(QColor neutral_color) { neutral_color_ = neutral_color; }

    qreal border_margin() const { return border_margin_; }
    void set_border_margin(qreal border_margin) { border_margin_ = border_margin; }
    qreal planet_base_radius() const { return planet_base_radius_; }
    void set_planet_base_radius(qreal planet_base_radius) { planet_base_radius_ = planet_base_radius; }
    qreal planet_radius_incr_per_ship_prod() const { return planet_radius_incr_per_ship_prod_; }
    void set_planet_radius_incr_per_ship_prod(qreal planet_radius_incr_per_ship_prod) {
      planet_radius_incr_per_ship_prod_ = planet_radius_incr_per_ship_prod;
    }
    qreal fleet_size() const { return fleet_size_; }
    void set_fleet_size(qreal fleet_size) { fleet_size_ = fleet_size; }

  protected:
    virtual void paintEvent(QPaintEvent* event);

  private:
    Q_DISABLE_COPY(MapWidget)

    // Internal drawing routines
    void compute_map_bounding_box_();
    QPointF compute_planet_location_in_widget_coordinates_(const team_planets::Planet& planet);

    void draw_planet_(QPainter& painter, const team_planets::Planet& planet);
    void draw_fleet_(QPainter& painter, const team_planets::Fleet& fleet);

    // The battle map and thread
    QMutex*             map_mutex_;
    team_planets::Map*  map_;
    BattleThread*       battle_thread_;

    // Different map colors and properties
    QColor  background_color_;
    QColor  neutral_color_;

    qreal   border_margin_;
    qreal   planet_base_radius_;
    qreal   planet_radius_incr_per_ship_prod_;
    qreal   fleet_size_;

    // Internal data
    QRectF  map_bounding_box_;
  };
}

#endif
