/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/accessor/accessor.h"
#include "models/common/vectorset.h"
#include <QObject>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {

// SmallTileTileset and LargeTileTileset are implemented manually because their
// container is not std::vector and the accessor edits FrameObject tileIds when
// the tileset is modified

class SmallTileTileset : public Accessor::AbstractListAccessor {
    Q_OBJECT

public:
    using DataT = Snes::Tile8px;
    using ListT = Snes::Tileset8px;
    using index_type = size_t;
    using ArgsT = std::tuple<>;

    using UndoHelper = Accessor::ListWithNoSelectionUndoHelper<SmallTileTileset>;

    constexpr static index_type maxSize() { return 256; }

private:
    ResourceItem* const _resourceItem;

public:
    SmallTileTileset(ResourceItem* resourceItem);

    ResourceItem* resourceItem() const { return _resourceItem; }

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    virtual bool listExists() const final;
    virtual size_t size() const final;

    bool addItem();
    virtual bool addItem(size_t index) final;
    virtual bool cloneItem(size_t index) final;
    virtual bool removeItem(size_t index) final;
    virtual bool moveItem(size_t from, size_t to) final;

    bool editTile_setPixel(unsigned tileId, const QPoint& p, unsigned c, bool first = false);
    bool editTile_paintPixel(unsigned tileId, const QPoint& p, bool first = false);

protected:
    template <class, class>
    friend class Accessor::ListUndoHelper;
    ListT* getList()
    {
        MS::FrameSet* fs = _resourceItem->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->smallTileset;
    }

    ArgsT selectedListTuple() const
    {
        return std::make_tuple();
    }
};

class LargeTileTileset : public Accessor::AbstractListAccessor {
    Q_OBJECT

public:
    using DataT = Snes::Tile16px;
    using ListT = Snes::TilesetTile16;
    using index_type = size_t;
    using ArgsT = std::tuple<>;

    using UndoHelper = Accessor::ListWithNoSelectionUndoHelper<LargeTileTileset>;

private:
    ResourceItem* const _resourceItem;

public:
    LargeTileTileset(ResourceItem* resourceItem);

    ResourceItem* resourceItem() const { return _resourceItem; }

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    virtual bool listExists() const final;
    virtual size_t size() const final;

    bool addItem();
    virtual bool addItem(size_t index) final;
    virtual bool cloneItem(size_t index) final;
    virtual bool removeItem(size_t index) final;
    virtual bool moveItem(size_t from, size_t to) final;

    bool editTile_setPixel(unsigned tileId, const QPoint& p, unsigned c, bool first = false);
    bool editTile_paintPixel(unsigned tileId, const QPoint& p, bool first = false);

protected:
    template <class, class>
    friend class Accessor::ListUndoHelper;
    ListT* getList()
    {
        MS::FrameSet* fs = _resourceItem->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->largeTileset;
    }

    ArgsT selectedListTuple() const
    {
        return std::make_tuple();
    }
};

class PaletteList : public Accessor::VectorSingleSelectionAccessor<UnTech::Snes::Palette4bpp, ResourceItem> {
    Q_OBJECT

private:
    unsigned _selectedColor;

public:
    PaletteList(ResourceItem* resourceItem);

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    index_type selectedColor() const { return _selectedColor; }
    void setSelectedColor(unsigned color);
    void unselectColor() { setSelectedColor(INT_MAX); }

    bool isSelectedColorValid() const;

    void editSelected_setColorDialog(unsigned colorIndex, QWidget* widget = nullptr);

    bool editSelectedList_addItem();
    bool editSelectedList_cloneSelected();
    bool editSelectedList_raiseSelected();
    bool editSelectedList_lowerSelected();
    bool editSelectedList_removeSelected();

signals:
    void selectedColorChanged();
};

class FrameList : public Accessor::NamedListAccessor<MS::Frame, ResourceItem> {
    Q_OBJECT

public:
    using UndoHelper = Accessor::ListAndSelectionUndoHelper<FrameList>;

    using SpriteOrderType = UnTech::MetaSprite::SpriteOrderType;

private:
    bool _tileHitboxSelected;

public:
    FrameList(ResourceItem* resourceItem);
    ~FrameList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool isTileHitboxSelected() const { return _tileHitboxSelected && isSelectedIndexValid(); }
    void setTileHitboxSelected(bool s);

    bool editSelected_setSpriteOrder(SpriteOrderType spriteOrder);
    bool editSelected_setSolid(bool solid);
    bool editSelected_toggleTileHitbox();

    // Will also edit frame.solid
    bool editSelected_setTileHitbox(const ms8rect& hitbox);

signals:
    void frameLocationChanged(index_type index);

    void tileHitboxSelectedChanged();
};

class FrameObjectList : public Accessor::ChildVectorMultipleSelectionAccessor<MS::FrameObject, ResourceItem> {
    Q_OBJECT

    using ObjectSize = UnTech::MetaSprite::ObjectSize;

public:
    FrameObjectList(ResourceItem* resourceItem);
    ~FrameObjectList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool editSelectedList_setLocation(unsigned index, const ms8point& location);
    bool editSelectedList_setSize(unsigned index, ObjectSize size);
    bool editSelectedList_setTile(unsigned index, unsigned tileId);
    bool editSelectedList_setFlips(unsigned index, bool hFlip, bool vFlip);

    bool editSelected_setTileIdAndSize(unsigned tileId, ObjectSize size);

    bool editSelected_toggleObjectSize();
    bool editSelected_flipObjectHorizontally();
    bool editSelected_flipObjectVertically();

protected:
    // shifts all tiles with a size of size and a tileId >= tileId by offset
    friend class SmallTileTileset;
    friend class LargeTileTileset;
    bool editAll_shiftTileIds(ObjectSize size, unsigned tileId, int offset);
};

class ActionPointList : public Accessor::ChildVectorMultipleSelectionAccessor<MS::ActionPoint, ResourceItem> {
    Q_OBJECT

public:
    ActionPointList(ResourceItem* resourceItem);
    ~ActionPointList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool editSelectedList_setLocation(unsigned index, const ms8point& location);
    bool editSelectedList_setType(unsigned index, idstring type);
};

class EntityHitboxList : public Accessor::ChildVectorMultipleSelectionAccessor<MS::EntityHitbox, ResourceItem> {
    Q_OBJECT

    using EntityHitboxType = UnTech::MetaSprite::EntityHitboxType;

public:
    EntityHitboxList(ResourceItem* resourceItem);
    ~EntityHitboxList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool editSelectedList_setAabb(unsigned index, const ms8rect& aabb);
    bool editSelectedList_setEntityHitboxType(unsigned index, EntityHitboxType type);

    bool editSelected_setEntityHitboxType(EntityHitboxType type);
};

}
}
}
}
