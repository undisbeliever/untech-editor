/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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
class Document;
class SiFrameGraphicsItem;

class SiGraphicsScene : public QGraphicsScene {
    Q_OBJECT

public:
    SiGraphicsScene(LayerSettings* layerSettings, QWidget* parent = nullptr);
    ~SiGraphicsScene() = default;

    void setDocument(Document* document);

private slots:
    void onLayerSettingsChanged();

    void updateFrameSetPixmap();

    void buildFrameItems();

    void onFrameSetGridChanged();
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
    LayerSettings* _layerSettings;
    Document* _document;

    Style* _style;
    QGraphicsPixmapItem* _frameSetPixmap;
    QMap<const void*, SiFrameGraphicsItem*> _frameItems;
};
}
}
}
}
