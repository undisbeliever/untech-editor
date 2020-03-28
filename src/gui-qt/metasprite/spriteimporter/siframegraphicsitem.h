/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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

namespace SI = UnTech::MetaSprite::SpriteImporter;

class SiFrameGraphicsItem : public AabbGraphicsItem {
public:
    static const unsigned FRAME_OBJECT_ZVALUE = 100;
    static const unsigned ENTITY_HITBOX_ZVALUE = 200;
    static const unsigned TILE_HITBOX_ZVALUE = 300;
    static const unsigned ACTION_POINT_ZVALUE = 400;
    static const unsigned ORIGIN_ZVALUE = 500;

public:
    SiFrameGraphicsItem(const SI::Frame& frame, QMenu* contextMenu, Style* style,
                        QGraphicsItem* parent = nullptr);
    ~SiFrameGraphicsItem() = default;

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

    void updateFrameLocation(const SI::Frame& frame);
    void onFrameDataChanged(const SI::Frame& frame);

    void updateLayerSettings(const SI::Frame& frame, const LayerSettings* settings);

    void updateFrameObject(size_t index, const SI::FrameObject& obj);
    void updateActionPoint(size_t index, const SI::ActionPoint& ap);
    void updateEntityHitbox(size_t index, const SI::EntityHitbox& eh);

    void onFrameObjectListChanged(const SI::Frame& frame);
    void onActionPointListChanged(const SI::Frame& frame);
    void onEntityHitboxListChanged(const SI::Frame& frame);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    void addFrameObject(const SI::Frame& frame);
    void addActionPoint(const SI::Frame& frame);
    void addEntityHitbox(const SI::Frame& frame);

private:
    QMenu* _contextMenu;
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
