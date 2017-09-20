/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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
        SQUARE,
        NTSC,
        PAL
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
    qreal zoom() const { return _zoom; }
    QString zoomString() const;

    void setAspectRatio(AspectRatio aspectRatio);
    AspectRatio aspectRatio() const { return _aspectRatio; }

    void populateMenu(QMenu* menu);

    void setZoomComboBox(QComboBox* comboBox);
    QComboBox* zoomComboBox() const { return _zoomComboBox; }

    void setAspectRatioComboBox(QComboBox* comboBox);
    QComboBox* aspectComboBox() const { return _aspectRatioComboBox; }

private:
    void updateTransform();

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();

private slots:
    void updateZoomComboBox();
    void updateAspectRatioComboBox();

    void onZoomComboBoxActivated(int index);
    void onZoomComboBoxEdited();
    void onZoomMenuTriggered(QAction* action);
    void onAspectRatioComboBoxActivated(int index);
    void onAspectRatioMenuTriggered(QAction* action);

signals:
    void zoomChanged();
    void aspectRatioChanged();
    void transformChanged();

private:
    qreal _zoom;
    AspectRatio _aspectRatio;
    QTransform _transform;

    QMenu* _aspectRatioMenu;
    QMenu* _zoomMenu;

    QRegExpValidator* _zoomValidator;

    QComboBox* _zoomComboBox;
    QComboBox* _aspectRatioComboBox;
};
}
}
