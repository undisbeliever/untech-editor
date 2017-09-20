/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frameobjecttilewidget.h"

#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;
using TilesetType = UnTech::MetaSprite::TilesetType;

const QStringList FrameObjectTileWidget::FLIP_STRINGS(
    { QString(),
      QString::fromUtf8("hFlip"),
      QString::fromUtf8("vFlip"),
      QString::fromUtf8("hvFlip") });

FrameObjectTileWidget::FrameObjectTileWidget(QWidget* parent)
    : QWidget(parent)
    , _nSmallTiles(512)
    , _nLargeTiles(512)
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(1, 1, 1, 1);

    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);

    this->setLayout(layout);

    _tileSize = new QComboBox(this);
    _tileSize->addItem(tr("Small"));
    _tileSize->addItem(tr("Large"));
    layout->addWidget(_tileSize, 0, 0);

    _tileId = new QSpinBox(this);
    layout->addWidget(_tileId, 0, 1);

    _tileFlip = new QComboBox(this);
    _tileFlip->addItems(FLIP_STRINGS);
    layout->addWidget(_tileFlip, 1, 0, 1, 2);

    this->setMinimumSize(layout->minimumSize());

    _tileSize->setFocusPolicy(Qt::WheelFocus);
    _tileId->setFocusPolicy(Qt::StrongFocus);
    _tileFlip->setFocusPolicy(Qt::StrongFocus);

    this->setTabOrder(_tileSize, _tileId);
    this->setTabOrder(_tileId, _tileFlip);

    updateTileIdRange();

    connect(_tileSize, qOverload<int>(&QComboBox::activated),
            this, &FrameObjectTileWidget::updateTileIdRange);

    _tileId->installEventFilter(this);
    _tileSize->installEventFilter(this);
    _tileFlip->installEventFilter(this);
}

void FrameObjectTileWidget::focusInEvent(QFocusEvent*)
{
    _tileSize->setFocus();
}

bool FrameObjectTileWidget::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        const auto& key = keyEvent->key();

        if (key == Qt::Key_Backtab && _tileSize->hasFocus()) {
            QApplication::sendEvent(this, event);
            return true;
        }
        else if (key == Qt::Key_Tab && _tileFlip->hasFocus()) {
            QApplication::sendEvent(this, event);
            return true;
        }
    }
    else if (event->type() == QEvent::FocusOut) {
        if (_tileSize->hasFocus() == false
            && _tileId->hasFocus() == false
            && _tileFlip->hasFocus() == false) {

            QApplication::sendEvent(this, event);
            return false;
        }
    }

    return QWidget::eventFilter(object, event);
}

unsigned FrameObjectTileWidget::extractValue(const MS::FrameObject& obj)
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    return (obj.tileId & 0xffff)
           | ((obj.size == ObjSize::LARGE) << SIZE_SHIFT)
           | (obj.hFlip << HFLIP_SHIFT)
           | (obj.vFlip << VFLIP_SHIFT);
}

void FrameObjectTileWidget::applyValue(MS::FrameObject& obj, unsigned value)
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    obj.tileId = value & 0xffff;
    obj.size = (value & (1 << SIZE_SHIFT)) ? ObjSize::LARGE : ObjSize::SMALL;
    obj.hFlip = value & (1 << HFLIP_SHIFT);
    obj.vFlip = value & (1 << VFLIP_SHIFT);
}

unsigned FrameObjectTileWidget::value() const
{
    return (_tileId->value() & 0xffff)
           | (_tileSize->currentIndex() << SIZE_SHIFT)
           | (_tileFlip->currentIndex() << FLIP_SHIFT);
}

void FrameObjectTileWidget::setValue(unsigned value)
{
    _tileSize->setCurrentIndex((value >> SIZE_SHIFT) & 1);

    updateTileIdRange();

    _tileId->setValue(value & 0xffff);
    _tileFlip->setCurrentIndex((value >> FLIP_SHIFT) & 3);
}

void FrameObjectTileWidget::setTilesetSize(const MS::FrameSet& frameSet)
{
    _nSmallTiles = frameSet.smallTileset.size();
    _nLargeTiles = frameSet.largeTileset.size();

    updateTileIdRange();
}

void FrameObjectTileWidget::updateTileIdRange()
{
    if (_tileSize->currentIndex() == 0) {
        _tileId->setMaximum(_nSmallTiles - 1);
    }
    else {
        _tileId->setMaximum(_nLargeTiles - 1);
    }
}
