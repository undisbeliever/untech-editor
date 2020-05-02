/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "zoomablegraphicsview.h"

class QSurfaceFormat;

namespace UnTech {
namespace GuiQt {

class OpenGLZoomableGraphicsView : public ZoomableGraphicsView {
    Q_OBJECT

public:
    OpenGLZoomableGraphicsView(QWidget* parent = nullptr);
    ~OpenGLZoomableGraphicsView() = default;

private:
    static QSurfaceFormat surfaceFormat();
};

}
}
