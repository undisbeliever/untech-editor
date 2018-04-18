/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/graphics/aabbgraphicsitem.h"
#include "models/common/vectorset.h"
#include "models/metasprite/spriteimporter.h"

#include <QGraphicsLineItem>
#include <QList>

namespace UnTech {
namespace GuiQt {
class AabbGraphicsItem;
class ResizableAabbGraphicsItem;

namespace MetaSprite {
class Style;
class LayerSettings;

namespace SpriteImporter {
class Actions;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class SiFrameGraphicsItem : public AabbGraphicsItem {
public:
    static const unsigned FRAME_OBJECT_ZVALUE = 100;
    static const unsigned ENTITY_HITBOX_ZVALUE = 200;
    static const unsigned TILE_HITBOX_ZVALUE = 300;
    static const unsigned ACTION_POINT_ZVALUE = 400;
    static const unsigned ORIGIN_ZVALUE = 500;

public:
    SiFrameGraphicsItem(SI::Frame* frame, Actions* actions, Style* style,
                        QGraphicsItem* parent = nullptr);
    ~SiFrameGraphicsItem() = default;

    const SI::Frame* frame() const { return _frame; }

    const auto* tileHitbox() const { return _tileHitbox; }
    const auto& objects() const { return _objects; }
    const auto& actionPoints() const { return _actionPoints; }
    const auto& entityHitboxes() const { return _entityHitboxes; }

    bool frameSelected() const { return _frameSelected; }
    void setFrameSelected(bool selected);

    void updateFrameObjectSelection(const vectorset<size_t>& selectedIndexes);
    void updateActionPointSelection(const vectorset<size_t>& selectedIndexes);
    void updateEntityHitboxSelection(const vectorset<size_t>& selectedIndexes);

    void updateTileHitboxSelected(bool s);

    void updateFrameLocation();
    void onFrameDataChanged();

    void updateLayerSettings(const LayerSettings* settings);

    void updateFrameObject(size_t index);
    void updateActionPoint(size_t index);
    void updateEntityHitbox(size_t index);

    void onFrameObjectListChanged();
    void onActionPointListChanged();
    void onEntityHitboxListChanged();

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    void addFrameObject();
    void addActionPoint();
    void addEntityHitbox();

private:
    const SI::Frame* _frame;
    Actions* _actions;
    Style* _style;
    bool _showTileHitbox;
    bool _frameSelected;

    ResizableAabbGraphicsItem* _tileHitbox;
    QGraphicsLineItem* _horizontalOrigin;
    QGraphicsLineItem* _verticalOrigin;

    QList<AabbGraphicsItem*> _objects;
    QList<AabbGraphicsItem*> _actionPoints;
    QList<ResizableAabbGraphicsItem*> _entityHitboxes;
};
}
}
}
}
