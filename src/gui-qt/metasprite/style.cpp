/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "style.h"
#include <QBrush>
#include <QPen>

using namespace UnTech::GuiQt::MetaSprite;

const QColor Style::ANTI_HIGHLIGHT_BRUSH_COLOR(128, 128, 128, 128);

const QColor Style::FRAME_OUTLINE_PEN_COLOR(160, 160, 160, 240);
const QColor Style::ORIGIN_PEN_COLOR(160, 160, 160, 240);
const QColor Style::PALETTE_OUTLINE_COLOR(160, 160, 160, 240);

const QColor Style::TILE_HITBOX_PEN_COLOR(192, 0, 0, 240);
const QColor Style::FRAME_OBJECT_PEN_COLOR(64, 128, 64, 240);
const QColor Style::ACTION_POINT_PEN_COLOR(192, 192, 192, 240);
const QColor Style::ENTITY_HITBOX_PEN_COLOR(0, 0, 255, 240);

const QColor Style::TILE_HITBOX_BRUSH_COLOR(192, 0, 0, 32);
const QColor Style::ACTION_POINT_BRUSH_COLOR(192, 192, 192, 128);
const QColor Style::ENTITY_HITBOX_BRUSH_COLOR(0, 0, 255, 32);

const QColor Style::EH_BODY_BRUSH_COLOR(0, 0, 255, 32);
const QColor Style::EH_BODY_WEAK_BRUSH_COLOR(123, 123, 255, 32);
const QColor Style::EH_BODY_ATTACK_BRUSH_COLOR(123, 0, 165, 32);
const QColor Style::EH_SHIELD_BRUSH_COLOR(181, 181, 0, 32);
const QColor Style::EH_SHIELD_ATTACK_BRUSH_COLOR(222, 82, 0, 32);
const QColor Style::EH_ATTACK_BRUSH_COLOR(198, 0, 0, 32);

Style::Style(QWidget* parent)
    : QObject(parent)
    , _widget(parent)
{
    Q_ASSERT(_widget != nullptr);
}

QPen Style::createCosmeticPen(const QColor& color) const
{
#if QT_VERSION >= 0x050600
    qreal pixelRatio = _widget->devicePixelRatioF();
#else
    int pixelRatio = _widget->devicePixelRatio();
#endif

    QPen pen(color, pixelRatio);
    pen.setCosmetic(true);
    return pen;
}

QPen Style::frameOutlinePen() const
{
    return createCosmeticPen(FRAME_OUTLINE_PEN_COLOR);
}

QPen Style::originPen() const
{
    return QPen(ORIGIN_PEN_COLOR, 0.0, Qt::DashLine);
}

QPen Style::paletteOutlinePen() const
{
    return createCosmeticPen(PALETTE_OUTLINE_COLOR);
}

QPen Style::tileHitboxPen() const
{
    return createCosmeticPen(TILE_HITBOX_PEN_COLOR);
}

QPen Style::frameObjectPen() const
{
    return createCosmeticPen(FRAME_OBJECT_PEN_COLOR);
}

QPen Style::actionPointPen() const
{
    return createCosmeticPen(ACTION_POINT_PEN_COLOR);
}

QPen Style::entityHitboxPen(const EntityHitboxType&) const
{
    return createCosmeticPen(ENTITY_HITBOX_PEN_COLOR);
}

QBrush Style::antiHighlightBrush() const
{
    return QBrush(ANTI_HIGHLIGHT_BRUSH_COLOR);
}

QBrush Style::tileHitboxBrush() const
{
    return QBrush(TILE_HITBOX_BRUSH_COLOR);
}

QBrush Style::actionPointBrush() const
{
    return QBrush(ACTION_POINT_BRUSH_COLOR);
}

QBrush Style::entityHitboxBrush(const EntityHitboxType&) const
{
    // ::TODO different brush depending on type::
    return QBrush(ENTITY_HITBOX_BRUSH_COLOR);
}
