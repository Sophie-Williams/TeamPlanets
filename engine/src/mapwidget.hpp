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
namespace team_planets { class Map; }

namespace team_planets_engine {
  class MapWidget: public QWidget {
    Q_OBJECT
    Q_PROPERTY(QColor background_color READ background_color WRITE set_background_color)

  public:
    explicit MapWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

    void set_map(QMutex& map_mutex, team_planets::Map& map) {
      map_mutex_ = &map_mutex;
      map_ = &map;
    }

    // Different map colors accessors
    QColor background_color() const { return background_color_; }
    void set_background_color(QColor background_color) { background_color_ = background_color; }

  protected:
    virtual void paintEvent(QPaintEvent* event);

  private:
    Q_DISABLE_COPY(MapWidget)

    QMutex*             map_mutex_;
    team_planets::Map*  map_;

    // Different map colors
    QColor  background_color_;
  };
}

#endif
