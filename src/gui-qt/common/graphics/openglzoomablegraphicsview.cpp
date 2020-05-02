/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "openglzoomablegraphicsview.h"

#include <QOpenGLWidget>

using namespace UnTech::GuiQt;

// Use the same format for all views
QSurfaceFormat OpenGLZoomableGraphicsView::surfaceFormat()
{
    const static QSurfaceFormat format = []() {
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();

        // Disable depth and stencil buffers
        format.setDepthBufferSize(0);
        format.setStencilBufferSize(0);

        return format;
    }();

    return format;
}

OpenGLZoomableGraphicsView::OpenGLZoomableGraphicsView(QWidget* parent)
    : ZoomableGraphicsView(parent)
{
    auto* glWidget = new QOpenGLWidget(this);
    glWidget->setFormat(surfaceFormat());
    setViewport(glWidget);
}
