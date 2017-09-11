/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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
    , _zoomValidator(new QRegExpValidator(ZOOM_REGEXP, this))
    , _zoomComboBox(nullptr)
    , _aspectRatioComboBox(nullptr)
{
    updateTransform();

    _zoomMenu = new QMenu(tr("Zoom"));
    QActionGroup* zoomActionGroup = new QActionGroup(this);
    zoomActionGroup->setExclusive(true);

    for (qreal z : ZOOM_LEVELS) {
        QAction* a = zoomActionGroup->addAction(QString::fromUtf8("%1%").arg(z * 100));
        a->setData(z);
        a->setCheckable(true);
        a->setChecked(z == _zoom);
        _zoomMenu->addAction(a);
    }

    _aspectRatioMenu = new QMenu(tr("Aspect Ratio"));
    QActionGroup* aspectRatioGroup = new QActionGroup(this);
    aspectRatioGroup->setExclusive(true);

    auto addAspectAction = [&](const char* title, AspectRatio ratio) {
        QAction* a = aspectRatioGroup->addAction(tr(title));
        a->setData(ratio);
        a->setCheckable(true);
        a->setChecked(ratio == _aspectRatio);
        _aspectRatioMenu->addAction(a);
    };
    addAspectAction("Square", SQUARE);
    addAspectAction("NTSC", NTSC);
    addAspectAction("PAL", PAL);

    connect(_zoomMenu, &QMenu::triggered,
            this, &ZoomSettings::onZoomMenuTriggered);

    connect(_aspectRatioMenu, &QMenu::triggered,
            this, &ZoomSettings::onAspectRatioMenuTriggered);
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

        updateZoomComboBox();
        updateTransform();

        emit zoomChanged();
    }

    for (QAction* a : _zoomMenu->actions()) {
        a->setChecked(a->data() == _zoom);
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

        for (QAction* a : _aspectRatioMenu->actions()) {
            if (a->data() == aspectRatio) {
                a->setChecked(true);
                break;
            }
        }

        updateAspectRatioComboBox();
        updateTransform();

        emit aspectRatioChanged();
    }
}

void ZoomSettings::populateMenu(QMenu* menu)
{
    menu->addMenu(_zoomMenu);
    menu->addMenu(_aspectRatioMenu);

    menu->addAction(QIcon(":/icons/zoom-in.svg"), tr("Zoom In"),
                    this, &ZoomSettings::zoomIn,
                    Qt::CTRL + Qt::Key_Plus);
    menu->addAction(QIcon(":/icons/zoom-out.svg"), tr("Zoom Out"),
                    this, &ZoomSettings::zoomOut,
                    Qt::CTRL + Qt::Key_Minus);
}

void ZoomSettings::setZoomComboBox(QComboBox* comboBox)
{
    if (_zoomComboBox == comboBox) {
        return;
    }

    if (_zoomComboBox) {
        _zoomComboBox->setValidator(nullptr);
        _zoomComboBox->disconnect(this);
        if (QLineEdit* w = _zoomComboBox->lineEdit()) {
            w->disconnect(this);
        }
    }
    _zoomComboBox = comboBox;

    if (_zoomComboBox) {
        _zoomComboBox->clear();
        for (qreal z : ZOOM_LEVELS) {
            QString s = QString::fromUtf8("%1%").arg(z * 100);
            _zoomComboBox->addItem(s, z);
        }

        _zoomComboBox->setEditable(true);
        _zoomComboBox->setInsertPolicy(QComboBox::NoInsert);
        _zoomComboBox->setValidator(_zoomValidator);

        updateZoomComboBox();

        connect(_zoomComboBox, qOverload<int>(&QComboBox::activated),
                this, &ZoomSettings::onZoomComboBoxActivated);
        connect(_zoomComboBox->lineEdit(), &QLineEdit::editingFinished,
                this, &ZoomSettings::onZoomComboBoxEdited);
    }
}

void ZoomSettings::setAspectRatioComboBox(QComboBox* comboBox)
{
    if (_aspectRatioComboBox == comboBox) {
        return;
    }

    if (_aspectRatioComboBox) {
        _aspectRatioComboBox->disconnect(this);
    }
    _aspectRatioComboBox = comboBox;

    if (_aspectRatioComboBox) {
        _aspectRatioComboBox->clear();
        _aspectRatioComboBox->addItem(tr("Square"), SQUARE);
        _aspectRatioComboBox->addItem(tr("NTSC"), NTSC);
        _aspectRatioComboBox->addItem(tr("PAL"), PAL);

        updateAspectRatioComboBox();

        connect(_aspectRatioComboBox, qOverload<int>(&QComboBox::activated),
                this, &ZoomSettings::onAspectRatioComboBoxActivated);
    }
}

void ZoomSettings::updateZoomComboBox()
{
    if (_zoomComboBox) {
        int index = _zoomComboBox->findData(_zoom);
        _zoomComboBox->setCurrentIndex(index);
        if (index == -1) {
            _zoomComboBox->setEditText(zoomString());
        }
    }
}

void ZoomSettings::onZoomComboBoxActivated(int index)
{
    qreal z = _zoomComboBox->itemData(index).toDouble();
    if (z > 0) {
        setZoom(z);
    }
    else {
        resetZoom();
    }
}

void ZoomSettings::onZoomComboBoxEdited()
{
    QRegExp m(ZOOM_REGEXP);

    if (m.exactMatch(_zoomComboBox->currentText())) {
        qreal zp = m.cap(1).toDouble();
        if (zp > 0) {
            setZoom(qBound(MIN_ZOOM, zp / 100.0, MAX_ZOOM));
        }
        else {
            updateZoomComboBox();
        }
    }
}

void ZoomSettings::onZoomMenuTriggered(QAction* action)
{
    qreal z = action->data().toDouble();
    if (z > 0) {
        setZoom(z);
    }
    else {
        resetZoom();
    }
}

void ZoomSettings::updateAspectRatioComboBox()
{
    if (_aspectRatioComboBox) {
        int index = _aspectRatioComboBox->findData(_aspectRatio);
        _aspectRatioComboBox->setCurrentIndex(index);
    }
}

void ZoomSettings::onAspectRatioComboBoxActivated(int index)
{
    int a = _aspectRatioComboBox->itemData(index).toInt();
    setAspectRatio(AspectRatio(a));
}

void ZoomSettings::onAspectRatioMenuTriggered(QAction* action)
{
    int a = action->data().toInt();
    setAspectRatio(AspectRatio(a));
}
