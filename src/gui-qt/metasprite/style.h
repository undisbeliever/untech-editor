/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/entityhitboxtype.h"
#include <QWidget>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {

class Style : public QObject {
    Q_OBJECT

    using EntityHitboxType = UnTech::MetaSprite::EntityHitboxType;

private:
    static const QColor ANTI_HIGHLIGHT_BRUSH_COLOR;

    static const QColor ORIGIN_PEN_COLOR;
    static const QColor PALETTE_OUTLINE_COLOR;

    static const QColor FRAME_OUTLINE_PEN_COLOR;
    static const QColor TILE_HITBOX_PEN_COLOR;
    static const QColor FRAME_OBJECT_PEN_COLOR;
    static const QColor ACTION_POINT_PEN_COLOR;
    static const QColor EH_BODY_PEN_COLOR;
    static const QColor EH_BODY_WEAK_PEN_COLOR;
    static const QColor EH_BODY_ATTACK_PEN_COLOR;
    static const QColor EH_SHIELD_PEN_COLOR;
    static const QColor EH_SHIELD_ATTACK_PEN_COLOR;
    static const QColor EH_ATTACK_PEN_COLOR;

    static const QColor TILE_HITBOX_BRUSH_COLOR;
    static const QColor ACTION_POINT_BRUSH_COLOR;
    static const QColor EH_BODY_BRUSH_COLOR;
    static const QColor EH_BODY_WEAK_BRUSH_COLOR;
    static const QColor EH_BODY_ATTACK_BRUSH_COLOR;
    static const QColor EH_SHIELD_BRUSH_COLOR;
    static const QColor EH_SHIELD_ATTACK_BRUSH_COLOR;
    static const QColor EH_ATTACK_BRUSH_COLOR;

public:
    Style(QWidget* parent);
    ~Style() = default;

    QPen frameOutlinePen() const;
    QPen originPen() const;
    QPen paletteOutlinePen() const;
    QPen tileHitboxPen() const;
    QPen frameObjectPen() const;
    QPen actionPointPen() const;
    QPen entityHitboxPen(const EntityHitboxType&) const;

    QBrush antiHighlightBrush() const;

    QBrush tileHitboxBrush() const;
    QBrush actionPointBrush() const;
    QBrush entityHitboxBrush(const EntityHitboxType&) const;

private:
    QPen createCosmeticPen(const QColor& color) const;

private:
    QWidget* const _widget;
};
}
}
}
