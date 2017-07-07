/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "gui-qt/metasprite/abstractselection.h"
#include "models/metasprite/spriteimporter.h"

#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QList>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class Style;
class LayerSettings;

namespace SpriteImporter {

namespace SI = UnTech::MetaSprite::SpriteImporter;

class SiFrameGraphicsItem : public QGraphicsRectItem {
public:
    static const unsigned FRAME_OBJECT_ZVALUE = 100;
    static const unsigned ENTITY_HITBOX_ZVALUE = 200;
    static const unsigned TILE_HITBOX_ZVALUE = 300;
    static const unsigned ACTION_POINT_ZVALUE = 400;
    static const unsigned ORIGIN_ZVALUE = 500;

    static const int SELECTION_ID = 0;

public:
    SiFrameGraphicsItem(SI::Frame* frame, Style* style,
                        QGraphicsItem* parent = nullptr);
    ~SiFrameGraphicsItem() = default;

    const SI::Frame* frame() const { return _frame; }

    bool frameSelected() const { return _frameSelected; }
    void setFrameSelected(bool selected);

    void updateSelection(const std::set<SelectedItem>& selection);

    void updateFrameLocation();
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

    void updateFrameContents();

    void updateLayerSettings(const LayerSettings* settings);

private:
    template <class T>
    static void updateItemIndexes(QList<T*>& list, unsigned start,
                                  unsigned baseZValue,
                                  const SelectedItem::Type& type);

private:
    SI::Frame* _frame;
    Style* _style;
    bool _showTileHitbox;
    bool _frameSelected;

    QGraphicsRectItem* _tileHitbox;
    QGraphicsLineItem* _horizontalOrigin;
    QGraphicsLineItem* _verticalOrigin;

    QList<QGraphicsRectItem*> _objects;
    QList<QGraphicsRectItem*> _actionPoints; // ::TODO change to cross::
    QList<QGraphicsRectItem*> _entityHitboxes;
};
}
}
}
}
