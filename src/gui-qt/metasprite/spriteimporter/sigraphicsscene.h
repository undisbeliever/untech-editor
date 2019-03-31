/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "siframegraphicsitem.h"
#include "models/common/vectorset.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMap>
#include <QMenu>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class Style;
class LayerSettings;

namespace SpriteImporter {
class Document;

class SiGraphicsScene : public QGraphicsScene {
    Q_OBJECT

    static const unsigned PIXMAP_ZVALUE = 100;
    static const unsigned FRAME_ZVALUE = 200;
    static const unsigned SELECTED_FRAME_ZVALUE = 250;
    static const unsigned PALETTE_ZVALUE = 300;

public:
    SiGraphicsScene(LayerSettings* layerSettings,
                    QWidget* parent = nullptr);
    ~SiGraphicsScene() = default;

    QMenu* frameContextMenu() { return _frameContextMenu.data(); }

    void setDocument(Document* document);

protected:
    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

private:
    void commitMovedItems();
    void removeAllFrameItems();

    template <typename F>
    void updateSelection(F f, const vectorset<size_t>& selectedIndexes);

signals:
    void frameContentSelected();

private slots:
    void onLayerSettingsChanged();

    void onSelectedFrameChanged();

    void updateFrameObjectSelection();
    void updateActionPointSelection();
    void updateEntityHitboxSelection();
    void updateTileHitboxSelection();

    void onSceneSelectionChanged();

    void updateFrameSetPixmap();
    void updatePaletteOutline();

    void buildFrameItems();

    void onFrameSetGridChanged();

    void onFrameAdded(size_t frameIndex);
    void onFrameAboutToBeRemoved(size_t frameIndex);

    void onFrameLocationChanged(size_t frameIndex);
    void onFrameDataChanged(size_t frameIndex);

    void onFrameObjectChanged(size_t frameIndex, size_t index);
    void onActionPointChanged(size_t frameIndex, size_t index);
    void onEntityHitboxChanged(size_t frameIndex, size_t index);

    void onFrameObjectListChanged(size_t frameIndex);
    void onActionPointListChanged(size_t frameIndex);
    void onEntityHitboxListChanged(size_t frameIndex);

private:
    LayerSettings* const _layerSettings;

    QScopedPointer<QMenu> const _frameContextMenu;
    Style* const _style;
    QGraphicsPixmapItem* const _frameSetPixmap;
    QGraphicsPathItem* const _paletteOutline;

    QList<SiFrameGraphicsItem*> _frameItems;

    Document* _document;

    bool _inUpdateSelection;
    bool _inOnSceneSelectionChanged;
};
}
}
}
}
