/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/metasprite.h"
#include <QComboBox>
#include <QSpinBox>
#include <QStringList>
#include <QWidget>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;

namespace MS = UnTech::MetaSprite::MetaSprite;

class FrameObjectTileWidget : public QWidget {
    Q_OBJECT

public:
    const static QStringList FLIP_STRINGS;
    static constexpr unsigned SIZE_SHIFT = 16;
    static constexpr unsigned FLIP_SHIFT = 17;
    static constexpr unsigned HFLIP_SHIFT = 17;
    static constexpr unsigned VFLIP_SHIFT = 18;

public:
    explicit FrameObjectTileWidget(QWidget* parent = nullptr);
    ~FrameObjectTileWidget() = default;

    static unsigned extractValue(const MS::FrameObject& obj);
    static void applyValue(MS::FrameObject& obj, unsigned value);

    unsigned value() const;
    void setValue(unsigned value);

    void setTilesetSize(const MS::FrameSet& frameSet);

public slots:
    void updateTileIdRange();

protected:
    virtual void focusInEvent(QFocusEvent*) override;
    virtual bool eventFilter(QObject* object, QEvent* event) override;

private:
    QComboBox* _tileSize;
    QSpinBox* _tileId;
    QComboBox* _tileFlip;

    unsigned _nSmallTiles;
    unsigned _nLargeTiles;
};
}
}
}
}
