/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QGraphicsView>

namespace UnTech {
namespace GuiQt {
class ZoomSettings;

class ZoomableGraphicsView : public QGraphicsView {
    Q_OBJECT

public:
    explicit ZoomableGraphicsView(QWidget* parent = nullptr);
    ZoomableGraphicsView(QGraphicsScene* scene, QWidget* parent = nullptr);
    ~ZoomableGraphicsView() = default;

    void setZoomSettings(ZoomSettings* zoomSettings);
    ZoomSettings* zoomSettings() const { return _zoomSettings; }

private slots:
    void onZoomSettingsChanged();

private:
    ZoomSettings* _zoomSettings;
};
}
}
