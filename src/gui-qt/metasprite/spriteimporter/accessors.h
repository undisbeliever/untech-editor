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
namespace SpriteImporter {

class FrameObjectList;
class ActionPointList;
class EntityHitboxList;

class FrameMap : public QObject {
    Q_OBJECT

public:
    using DataT = SI::Frame;
    using MapT = SI::Frame::map_t;

private:
    Document* const _document;

    idstring _selectedId;
    SI::Frame* _selectedItem;

    bool _tileHitboxSelected;

public:
    FrameMap(Document* document);
    ~FrameMap() = default;

    Document* resourceItem() const { return _document; }

    static QString typeName() { return tr("Frame"); }

    const idstring& selectedId() const { return _selectedId; }
    const SI::Frame* selectedItem() const { return _selectedItem; }
    const SI::Frame* selectedFrame() const { return _selectedItem; }

    bool isTileHitboxSelected() const { return _tileHitboxSelected && _selectedItem != nullptr; }
    void setTileHitboxSelected(bool s);

    const MapT* map()
    {
        const SI::FrameSet* fs = _document->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->frames;
    }

public slots:
    void setSelectedId(const idstring& id);

    void unselectItem();

protected:
    friend class Accessor::IdmapUndoHelper<FrameMap>;
    MapT* getMap()
    {
        SI::FrameSet* fs = _document->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->frames;
    }

    friend class AbstractFrameContentAccessor;
    SI::Frame* selectedItemEditable() const { return _selectedItem; }

signals:
    void dataChanged(const SI::Frame*);
    void frameLocationChanged(const SI::Frame* frame);

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
    using ArgsT = std::tuple<SI::Frame*>;
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
    using DataT = SI::FrameObject;
    using ListT = DataT::list_t;

    constexpr static index_type max_size = ListT::MAX_SIZE;

public:
    FrameObjectList(Document* document);
    ~FrameObjectList() = default;

    static QString typeName() { return tr("Frame Object"); }
    static QString typeNamePlural() { return tr("Frame Objects"); }

protected:
    friend class Accessor::ListUndoHelper<FrameObjectList>;
    friend class Accessor::ListActionHelper;
    ListT* getList(SI::Frame* frame)
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
    using DataT = SI::ActionPoint;
    using ListT = DataT::list_t;

    constexpr static index_type max_size = ListT::MAX_SIZE;

public:
    ActionPointList(Document* document);
    ~ActionPointList() = default;

    static QString typeName() { return tr("Action Point"); }
    static QString typeNamePlural() { return tr("Action Points"); }

protected:
    friend class Accessor::ListUndoHelper<ActionPointList>;
    friend class Accessor::ListActionHelper;
    ListT* getList(SI::Frame* frame)
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
    using DataT = SI::EntityHitbox;
    using ListT = DataT::list_t;

    constexpr static index_type max_size = ListT::MAX_SIZE;

public:
    EntityHitboxList(Document* document);
    ~EntityHitboxList() = default;

    static QString typeName() { return tr("Entity Hitbox"); }
    static QString typeNamePlural() { return tr("Entity Hitboxes"); }

protected:
    friend class Accessor::ListUndoHelper<EntityHitboxList>;
    friend class Accessor::ListActionHelper;
    ListT* getList(SI::Frame* frame)
    {
        if (frame == nullptr) {
            return nullptr;
        }
        return &frame->entityHitboxes;
    }
};

using FrameMapUndoHelper = Accessor::IdmapAndSelectionUndoHelper<FrameMap>;
using FrameObjectListUndoHelper = Accessor::ListAndMultipleSelectionUndoHelper<FrameObjectList>;
using ActionPointListUndoHelper = Accessor::ListAndMultipleSelectionUndoHelper<ActionPointList>;
using EntityHitboxListUndoHelper = Accessor::ListAndMultipleSelectionUndoHelper<EntityHitboxList>;
}
}
}
}
