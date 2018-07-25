/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QComboBox>
#include <QMenu>
#include <QObject>
#include <QRegExpValidator>
#include <QTransform>
#include <QVector>

namespace UnTech {
namespace GuiQt {

class ZoomSettings : public QObject {
    Q_OBJECT

public:
    enum AspectRatio {
        SQUARE = 0,
        NTSC = 1,
        PAL = 2,
    };

    const static QRegExp ZOOM_REGEXP;

    const static QVector<qreal> ZOOM_LEVELS;
    const static qreal MIN_ZOOM;
    const static qreal MAX_ZOOM;

public:
    explicit ZoomSettings(QObject* parent = nullptr);
    ZoomSettings(qreal zoom, AspectRatio aspectRatio, QObject* parent = nullptr);
    ~ZoomSettings() = default;

    const QTransform& transform() const { return _transform; }

    void setZoom(qreal z);
    void setZoom(const QString& string);
    qreal zoom() const { return _zoom; }
    QString zoomString() const;

    void setAspectRatio(AspectRatio aspectRatio);
    void setAspectRatioInt(int aspectRatioId);
    AspectRatio aspectRatio() const { return _aspectRatio; }
    int aspectRatioInt() const { return int(_aspectRatio); }

private:
    void updateTransform();

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void zoomChanged();
    void aspectRatioChanged();
    void transformChanged();

private:
    qreal _zoom;
    AspectRatio _aspectRatio;
    QTransform _transform;
};
}
}
