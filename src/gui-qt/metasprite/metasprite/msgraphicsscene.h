/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/vectorset.h"
#include "models/metasprite/metasprite.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>
#include <QMenu>

namespace UnTech {
namespace GuiQt {
class AabbGraphicsItem;
class PixmapGraphicsItem;
class ResizableAabbGraphicsItem;

namespace MetaSprite {
class Style;
class LayerSettings;

namespace MetaSprite {
class Document;
class TilesetPixmaps;

namespace MS = UnTech::MetaSprite::MetaSprite;

class MsGraphicsScene : public QGraphicsScene {
    Q_OBJECT

public:
    static const QRect ITEM_RANGE;

    static const unsigned FRAME_OBJECT_ZVALUE = 100;
    static const unsigned ENTITY_HITBOX_ZVALUE = 200;
    static const unsigned TILE_HITBOX_ZVALUE = 300;
    static const unsigned ACTION_POINT_ZVALUE = 400;
    static const unsigned ORIGIN_ZVALUE = 500;

public:
    MsGraphicsScene(LayerSettings* layerSettings,
                    TilesetPixmaps* tilesetPixmaps, QWidget* parent = nullptr);
    ~MsGraphicsScene() = default;

    QMenu* contextMenu() { return _contextMenu.data(); }

    void setDocument(Document* document);

    void setFrame(const UnTech::MetaSprite::MetaSprite::Frame* frame);

protected:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    void commitMovedItems();

    template <class T>
    void updateSelection(QList<T>& items,
                         const vectorset<size_t>& selectedIndexes);

    void updateTileHitbox();

    void addFrameObject();
    void updateFrameObject(unsigned index);

    void addActionPoint();
    void updateActionPoint(unsigned index);

    void addEntityHitbox();
    void updateEntityHitbox(unsigned index);

private slots:
    void onLayerSettingsChanged();

    void onSelectedFrameChanged();

    void updateFrameObjectSelection();
    void updateActionPointSelection();
    void updateEntityHitboxSelection();
    void updateTileHitboxSelection();

    void onSceneSelectionChanged();

    void onTilesetPixmapsChanged();

    void onFrameDataChanged(const void* frame);

    void onFrameObjectChanged(const void* frame, size_t index);
    void onActionPointChanged(const void* frame, size_t index);
    void onEntityHitboxChanged(const void* frame, size_t index);

    void onFrameObjectListChanged(const void* frame);
    void onActionPointListChanged(const void* frame);
    void onEntityHitboxListChanged(const void* frame);

private:
    LayerSettings* const _layerSettings;
    TilesetPixmaps* const _tilesetPixmaps;
    QScopedPointer<QMenu> const _contextMenu;
    Style* const _style;

    ResizableAabbGraphicsItem* const _tileHitbox;
    QGraphicsLineItem* const _horizontalOrigin;
    QGraphicsLineItem* const _verticalOrigin;

    QList<PixmapGraphicsItem*> _objects;
    QList<AabbGraphicsItem*> _actionPoints;
    QList<ResizableAabbGraphicsItem*> _entityHitboxes;

    Document* _document;
    const MS::Frame* _frame;

    bool _inUpdateSelection;
    bool _inOnSceneSelectionChanged;
};
}
}
}
}
