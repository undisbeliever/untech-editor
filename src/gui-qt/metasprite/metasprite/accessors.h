/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "document.h"
#include "gui-qt/accessor/accessor.h"
#include "models/common/vectorset.h"
#include <QObject>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {

class FrameObjectList;
class ActionPointList;
class EntityHitboxList;

class SmallTileTileset : public QObject {
    Q_OBJECT

public:
    constexpr static unsigned max_size = 256;

    using DataT = Snes::Tile8px;
    using ListT = Snes::Tileset8px;
    using index_type = unsigned;
    using ArgsT = std::tuple<>;

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
    constexpr static unsigned max_size = 256;

    using DataT = Snes::Tile16px;
    using ListT = Snes::TilesetTile16;
    using index_type = unsigned;
    using ArgsT = std::tuple<>;

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
    constexpr static size_t max_size = UnTech::MetaSprite::MAX_PALETTES;

    using DataT = UnTech::Snes::Palette4bpp;
    using ListT = std::vector<Snes::Palette4bpp>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<>;
    using SignalArgsT = ArgsT;

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

class FrameMap : public QObject {
    Q_OBJECT

public:
    using DataT = MS::Frame;
    using MapT = MS::Frame::map_t;

    using SpriteOrderType = UnTech::MetaSprite::SpriteOrderType;

private:
    Document* const _document;

    idstring _selectedId;
    MS::Frame* _selectedItem;

    bool _tileHitboxSelected;

public:
    FrameMap(Document* document);
    ~FrameMap() = default;

    Document* resourceItem() const { return _document; }

    static QString typeName() { return tr("Frame"); }

    const idstring& selectedId() const { return _selectedId; }
    const MS::Frame* selectedItem() const { return _selectedItem; }
    const MS::Frame* selectedFrame() const { return _selectedItem; }

    bool isFrameSelected() const { return _selectedItem != nullptr; }

    bool isTileHitboxSelected() const { return _tileHitboxSelected && _selectedItem != nullptr; }
    void setTileHitboxSelected(bool s);

    const MapT* map()
    {
        const MS::FrameSet* fs = _document->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->frames;
    }

    bool editSelected_setSpriteOrder(SpriteOrderType spriteOrder);
    bool editSelected_setSolid(bool solid);
    bool editSelected_setTileHitbox(const ms8rect& hitbox);
    bool editSelected_toggleTileHitbox();

public slots:
    void setSelectedId(const idstring& id);
    void unselectItem();

protected:
    friend class Accessor::IdmapUndoHelper<FrameMap>;
    MapT* getMap()
    {
        MS::FrameSet* fs = _document->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->frames;
    }

    friend class AbstractFrameContentAccessor;
    MS::Frame* selectedItemEditable() const { return _selectedItem; }

signals:
    void dataChanged(const MS::Frame*);
    void mapChanged();

    void mapAboutToChange();
    void itemAdded(const idstring& id);
    void itemAboutToBeRemoved(const idstring& id);
    void itemRenamed(const idstring& oldId, const idstring& newId);

    void selectedItemChanged();
    void tileHitboxSelectedChanged();
};

class AbstractFrameContentAccessor : public QObject {
    Q_OBJECT

public:
    using index_type = size_t;
    using ArgsT = std::tuple<MS::Frame*>;
    using SignalArgsT = std::tuple<const void*>;

private:
    Document* _document;

    vectorset<index_type> _selectedIndexes;

public:
    AbstractFrameContentAccessor(Document* document);
    ~AbstractFrameContentAccessor() = default;

    Document* resourceItem() const { return _document; }

    const vectorset<index_type>& selectedIndexes() const { return _selectedIndexes; }
    void setSelectedIndexes(const vectorset<index_type>& selected);
    void setSelectedIndexes(vectorset<index_type>&& selected);
    void clearSelection();

protected:
    friend class Accessor::ListUndoHelper<FrameObjectList>;
    friend class Accessor::ListUndoHelper<ActionPointList>;
    friend class Accessor::ListUndoHelper<EntityHitboxList>;
    friend class Accessor::MultipleSelectedIndexesHelper;
    ArgsT selectedListTuple() const
    {
        return std::make_tuple(_document->frameMap()->selectedItemEditable());
    }

signals:
    void dataChanged(const void* frame, index_type index);
    void listChanged(const void* frame);

    void listAboutToChange(const void* frame);
    void itemAdded(const void* frame, index_type index);
    void itemAboutToBeRemoved(const void* frame, index_type index);
    void itemMoved(const void* frame, index_type from, index_type to);

    void selectedListChanged();
    void selectedIndexesChanged();
};

class FrameObjectList : public AbstractFrameContentAccessor {
    Q_OBJECT

public:
    using DataT = MS::FrameObject;
    using ListT = std::vector<DataT>;
    constexpr static index_type max_size = UnTech::MetaSprite::MAX_FRAME_OBJECTS;

    using ObjectSize = UnTech::MetaSprite::ObjectSize;

public:
    FrameObjectList(Document* document);
    ~FrameObjectList() = default;

    static QString typeName() { return tr("Frame Object"); }
    static QString typeNamePlural() { return tr("Frame Objects"); }

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
    bool editSelectedList_shiftTiles(ObjectSize size, unsigned tileId, int offset);

protected:
    friend class Accessor::ListUndoHelper<FrameObjectList>;
    friend class Accessor::ListActionHelper;
    ListT* getList(MS::Frame* frame)
    {
        if (frame == nullptr) {
            return nullptr;
        }
        return &frame->objects;
    }
};

class ActionPointList : public AbstractFrameContentAccessor {
    Q_OBJECT

public:
    using DataT = MS::ActionPoint;
    using ListT = std::vector<DataT>;
    constexpr static index_type max_size = UnTech::MetaSprite::MAX_ACTION_POINTS;

    using ParameterType = UnTech::MetaSprite::ActionPointParameter;

public:
    ActionPointList(Document* document);
    ~ActionPointList() = default;

    static QString typeName() { return tr("Action Point"); }
    static QString typeNamePlural() { return tr("Action Points"); }

    bool editSelectedList_setLocation(unsigned index, const ms8point& location);
    bool editSelectedList_setParameter(unsigned index, ParameterType parameter);

protected:
    friend class Accessor::ListUndoHelper<ActionPointList>;
    friend class Accessor::ListActionHelper;
    ListT* getList(MS::Frame* frame)
    {
        if (frame == nullptr) {
            return nullptr;
        }
        return &frame->actionPoints;
    }
};

class EntityHitboxList : public AbstractFrameContentAccessor {
    Q_OBJECT

public:
    using DataT = MS::EntityHitbox;
    using ListT = std::vector<DataT>;
    constexpr static index_type max_size = UnTech::MetaSprite::MAX_ENTITY_HITBOXES;

    using EntityHitboxType = UnTech::MetaSprite::EntityHitboxType;

public:
    EntityHitboxList(Document* document);
    ~EntityHitboxList() = default;

    static QString typeName() { return tr("Entity Hitbox"); }
    static QString typeNamePlural() { return tr("Entity Hitboxes"); }

    bool editSelectedList_setAabb(unsigned index, const ms8rect& aabb);
    bool editSelectedList_setEntityHitboxType(unsigned index, EntityHitboxType type);

    bool editSelected_setEntityHitboxType(EntityHitboxType type);

protected:
    friend class Accessor::ListUndoHelper<EntityHitboxList>;
    friend class Accessor::ListActionHelper;
    ListT* getList(MS::Frame* frame)
    {
        if (frame == nullptr) {
            return nullptr;
        }
        return &frame->entityHitboxes;
    }
};

}
}
}
}
