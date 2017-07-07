/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/metasprite/abstractselection.h"
#include "models/metasprite/metasprite.h"
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QList>
#include <QMap>
#include <QWidget>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class Style;
class LayerSettings;

namespace MetaSprite {
class Document;
class SiFrameGraphicsItem;

namespace MS = UnTech::MetaSprite::MetaSprite;

class MsGraphicsScene : public QGraphicsScene {
    Q_OBJECT

    static const unsigned FRAME_OBJECT_ZVALUE = 100;
    static const unsigned ENTITY_HITBOX_ZVALUE = 200;
    static const unsigned TILE_HITBOX_ZVALUE = 300;
    static const unsigned ACTION_POINT_ZVALUE = 400;

    static const int SELECTION_ID = 0;

public:
    MsGraphicsScene(LayerSettings* layerSettings, QWidget* parent = nullptr);
    ~MsGraphicsScene() = default;

    void setDocument(Document* document);

    void setFrame(MS::Frame* frame);

protected:
    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;

private:
    template <class T>
    static void updateItemIndexes(QList<T*>& list, unsigned start,
                                  unsigned baseZValue,
                                  const SelectedItem::Type& type);

    void updateTileHitbox();

    void addFrameObject(unsigned index);
    void updateFrameObject(unsigned index);
    void removeFrameObject(unsigned index);

    void addActionPoint(unsigned index);
    void updateActionPoint(unsigned index);
    void removeActionPoint(unsigned index);

    void addEntityHitbox(unsigned index);
    void updateEntityHitbox(unsigned index);
    void removeEntityHitbox(unsigned index);

private slots:
    void onLayerSettingsChanged();

    void onSelectedFrameChanged();

    void updateSelection();
    void onSceneSelectionChanged();

    void onFrameTileHitboxChanged(const void* frame);

    void onFrameObjectChanged(const void* frame, unsigned index);
    void onActionPointChanged(const void* frame, unsigned index);
    void onEntityHitboxChanged(const void* frame, unsigned index);

    void onFrameObjectAboutToBeRemoved(const void* frame, unsigned index);
    void onActionPointAboutToBeRemoved(const void* frame, unsigned index);
    void onEntityHitboxAboutToBeRemoved(const void* frame, unsigned index);

    void onFrameObjectAdded(const void* frame, unsigned index);
    void onActionPointAdded(const void* frame, unsigned index);
    void onEntityHitboxAdded(const void* frame, unsigned index);

    void onFrameContentsMoved(const void* frame, const std::set<SelectedItem>& oldPositions, int offset);

private:
    LayerSettings* _layerSettings;
    Document* _document;

    Style* _style;
    MS::Frame* _frame;

    QGraphicsRectItem* _tileHitbox;

    QList<QGraphicsRectItem*> _objects;      // ::TODO change to FrameObjectItem::
    QList<QGraphicsRectItem*> _actionPoints; // ::TODO change to cross::
    QList<QGraphicsRectItem*> _entityHitboxes;

    bool _inUpdateSelection;
};
}
}
}
}
