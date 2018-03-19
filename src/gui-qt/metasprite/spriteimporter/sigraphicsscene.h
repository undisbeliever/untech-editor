/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/metasprite/abstractselection.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMap>
#include <QWidget>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class Style;
class LayerSettings;

namespace SpriteImporter {
class Actions;
class Document;
class SiFrameGraphicsItem;

class SiGraphicsScene : public QGraphicsScene {
    Q_OBJECT

    static const unsigned PIXMAP_ZVALUE = 100;
    static const unsigned FRAME_ZVALUE = 200;
    static const unsigned SELECTED_FRAME_ZVALUE = 250;
    static const unsigned PALETTE_ZVALUE = 300;

public:
    SiGraphicsScene(Actions* actions, LayerSettings* layerSettings,
                    QWidget* parent = nullptr);
    ~SiGraphicsScene() = default;

    void setDocument(Document* document);

protected:
    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

private:
    void commitMovedItems();
    void removeAllFrameItems();

private slots:
    void onLayerSettingsChanged();

    void onSelectedFrameChanged();
    void updateSelection();
    void onSceneSelectionChanged();

    void updateFrameSetPixmap();
    void updatePaletteOutline();

    void buildFrameItems();

    void onFrameSetGridChanged();

    void onFrameAdded(const void* frame);
    void onFrameAboutToBeRemoved(const void* frame);
    void onFrameLocationChanged(const void* frame);
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
    Actions* _actions;
    LayerSettings* _layerSettings;
    Document* _document;

    Style* _style;
    QGraphicsPixmapItem* _frameSetPixmap;
    QGraphicsPathItem* _paletteOutline;
    QMap<const void*, SiFrameGraphicsItem*> _frameItems;

    bool _inUpdateSelection;
};
}
}
}
}
