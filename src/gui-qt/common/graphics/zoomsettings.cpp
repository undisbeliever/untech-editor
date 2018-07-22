/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "zoomsettings.h"

#include <QLineEdit>

using namespace UnTech::GuiQt;

const QRegExp ZoomSettings::ZOOM_REGEXP("^\\s*([1-9][0-9]+)\\s*%?$");

const QVector<qreal> ZoomSettings::ZOOM_LEVELS = {
    1.0,
    1.5,
    2.0,
    3.0,
    4.0,
    5.0,
    6.0,
    7.0,
    8.0,
    9.0,
    12.0,
    15.0
};

const qreal ZoomSettings::MIN_ZOOM = ZOOM_LEVELS.first();
const qreal ZoomSettings::MAX_ZOOM = ZOOM_LEVELS.last();

ZoomSettings::ZoomSettings(QObject* parent)
    : ZoomSettings(1.0, SQUARE, parent)
{
}

ZoomSettings::ZoomSettings(qreal zoom, AspectRatio aspectRatio, QObject* parent)
    : QObject(parent)
    , _zoom(zoom)
    , _aspectRatio(aspectRatio)
    , _transform()
{
    updateTransform();
}

void ZoomSettings::updateTransform()
{
    // Values taken from bsnes-plus
    constexpr qreal NTSC_RATIO = 54.0 / 47.0;
    constexpr qreal PAL_RATIO = 32.0 / 23.0;

    qreal ratio = 1.0;
    if (_aspectRatio == NTSC) {
        ratio = NTSC_RATIO;
    }
    else if (_aspectRatio == PAL) {
        ratio = PAL_RATIO;
    }

    _transform = QTransform::fromScale(_zoom * ratio, _zoom);
    emit transformChanged();
}

void ZoomSettings::setZoom(qreal z)
{
    if (_zoom != z) {
        _zoom = z;

        updateTransform();

        emit zoomChanged();
    }
}

void ZoomSettings::setZoom(const QString& string)
{
    QRegExp m(ZOOM_REGEXP);
    if (m.exactMatch(string)) {
        qreal zp = m.cap(1).toDouble();
        if (zp > 0) {
            setZoom(qBound(MIN_ZOOM, zp / 100.0, MAX_ZOOM));
        }
    }
}

void ZoomSettings::zoomIn()
{
    qreal nextZoom = MAX_ZOOM;
    for (qreal z : ZOOM_LEVELS) {
        if (z > _zoom) {
            nextZoom = z;
            break;
        }
    }
    setZoom(nextZoom);
}

void ZoomSettings::zoomOut()
{
    qreal nextZoom = MIN_ZOOM;
    for (qreal z : ZOOM_LEVELS) {
        if (z >= _zoom) {
            break;
        }
        nextZoom = z;
    }
    setZoom(nextZoom);
}

void ZoomSettings::resetZoom()
{
    setZoom(1.0);
}

QString ZoomSettings::zoomString() const
{
    return QString::fromUtf8("%1%").arg(_zoom * 100);
}

void ZoomSettings::setAspectRatio(AspectRatio aspectRatio)
{
    if (_aspectRatio != aspectRatio) {
        _aspectRatio = aspectRatio;

        updateTransform();

        emit aspectRatioChanged();
    }
}
