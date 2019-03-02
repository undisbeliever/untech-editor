/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "document.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/accessor/accessor.h"
#include "models/common/vectorset.h"
#include <QObject>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {

class SmallTileTileset : public QObject {
    Q_OBJECT

public:
    using DataT = Snes::Tile8px;
    using ListT = Snes::Tileset8px;
    using index_type = unsigned;
    using ArgsT = std::tuple<>;

    constexpr static index_type maxSize() { return 256; }

private:
    Document* const _document;

public:
    SmallTileTileset(Document* document);

    Document* resourceItem() const { return _document; }

    static QString typeName() { return tr("Small Tile"); }

    bool editTileset_addTile();
    bool editTileset_addTile(unsigned index);
    bool editTileset_cloneTile(unsigned index);
    bool editTileset_removeTile(unsigned index);

    bool editTile_setPixel(unsigned tileId, const QPoint& p, unsigned c, bool first = false);
    bool editTile_paintPixel(unsigned tileId, const QPoint& p, bool first = false);

protected:
    friend class Accessor::ListUndoHelper<SmallTileTileset>;
    ListT* getList()
    {
        MS::FrameSet* fs = _document->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->smallTileset;
    }

    ArgsT selectedListTuple() const
    {
        return std::make_tuple();
    }

signals:
    void dataChanged(index_type index);
    void listChanged();

    void listAboutToChange();
    void itemAdded(unsigned index);
    void itemAboutToBeRemoved(unsigned index);
    void itemMoved(unsigned from, unsigned to);
};

class LargeTileTileset : public QObject {
    Q_OBJECT

public:
    using DataT = Snes::Tile16px;
    using ListT = Snes::TilesetTile16;
    using index_type = unsigned;
    using ArgsT = std::tuple<>;

    constexpr static index_type maxSize() { return 256; }

private:
    Document* const _document;

public:
    LargeTileTileset(Document* document);

    Document* resourceItem() const { return _document; }

    static QString typeName() { return tr("Large Tile"); }

    bool editTileset_addTile();
    bool editTileset_addTile(unsigned index);
    bool editTileset_cloneTile(unsigned index);
    bool editTileset_removeTile(unsigned index);

    bool editTile_setPixel(unsigned tileId, const QPoint& p, unsigned c, bool first = false);
    bool editTile_paintPixel(unsigned tileId, const QPoint& p, bool first = false);

protected:
    friend class Accessor::ListUndoHelper<LargeTileTileset>;
    ListT* getList()
    {
        MS::FrameSet* fs = _document->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->largeTileset;
    }

    ArgsT selectedListTuple() const
    {
        return std::make_tuple();
    }

signals:
    void dataChanged(index_type index);
    void listChanged();

    void listAboutToChange();
    void itemAdded(unsigned index);
    void itemAboutToBeRemoved(unsigned index);
    void itemMoved(unsigned from, unsigned to);
};

class PaletteList : public QObject {
    Q_OBJECT

public:
    using DataT = UnTech::Snes::Palette4bpp;
    using ListT = std::vector<Snes::Palette4bpp>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<>;
    using SignalArgsT = ArgsT;

    constexpr static index_type maxSize() { return UnTech::MetaSprite::MAX_PALETTES; }

private:
    Document* const _document;

    index_type _selectedIndex;
    unsigned _selectedColor;

public:
    PaletteList(Document* document);

    Document* resourceItem() const { return _document; }

    static QString typeName() { return tr("Palette"); }

    index_type selectedIndex() const { return _selectedIndex; }
    void setSelectedIndex(index_type index);
    void unselectItem() { setSelectedIndex(INT_MAX); }

    bool isSelectedIndexValid() const;

    index_type selectedColor() const { return _selectedColor; }
    void setSelectedColor(unsigned color);
    void unselectColor() { setSelectedColor(INT_MAX); }

    bool isSelectedColorValid() const;

    const ListT* palettes() const;
    const DataT* selectedPalette() const;

    void editSelected_setColorDialog(unsigned colorIndex, QWidget* widget = nullptr);

    bool editSelectedList_addItem();
    bool editSelectedList_cloneSelected();
    bool editSelectedList_raiseSelected();
    bool editSelectedList_lowerSelected();
    bool editSelectedList_removeSelected();

protected:
    friend class Accessor::ListUndoHelper<PaletteList>;
    friend class Accessor::ListActionHelper;
    friend class Accessor::SelectedIndexHelper;
    ListT* getList()
    {
        MS::FrameSet* fs = _document->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->palettes;
    }

    ArgsT selectedListTuple() const
    {
        return std::make_tuple();
    }

signals:
    void dataChanged(index_type index);
    void listChanged();

    void listAboutToChange();
    void itemAdded(index_type index);
    void itemAboutToBeRemoved(index_type index);
    void itemMoved(index_type from, index_type to);

    void selectedIndexChanged();
    void selectedColorChanged();
};

class FrameList : public Accessor::NamedListAccessor<MS::Frame, Document> {
    Q_OBJECT

    friend class Accessor::NamedListUndoHelper<FrameList>;
    using SpriteOrderType = UnTech::MetaSprite::SpriteOrderType;

private:
    bool _tileHitboxSelected;

public:
    FrameList(Document* document);
    ~FrameList() = default;

    virtual QString typeName() const final;

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

class FrameObjectList : public Accessor::ChildVectorMultipleSelectionAccessor<MS::FrameObject, Document> {
    Q_OBJECT

    friend class Accessor::ListUndoHelper<FrameObjectList>;

    using ObjectSize = UnTech::MetaSprite::ObjectSize;

public:
    FrameObjectList(Document* document);
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

class ActionPointList : public Accessor::ChildVectorMultipleSelectionAccessor<MS::ActionPoint, Document> {
    Q_OBJECT

    friend class Accessor::ListUndoHelper<ActionPointList>;

    using ParameterType = UnTech::MetaSprite::ActionPointParameter;

public:
    ActionPointList(Document* document);
    ~ActionPointList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool editSelectedList_setLocation(unsigned index, const ms8point& location);
    bool editSelectedList_setParameter(unsigned index, ParameterType parameter);
};

class EntityHitboxList : public Accessor::ChildVectorMultipleSelectionAccessor<MS::EntityHitbox, Document> {
    Q_OBJECT

    friend class Accessor::ListUndoHelper<EntityHitboxList>;

    using EntityHitboxType = UnTech::MetaSprite::EntityHitboxType;

public:
    EntityHitboxList(Document* document);
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
