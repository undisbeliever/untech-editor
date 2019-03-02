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
namespace SpriteImporter {

class FrameObjectList;
class ActionPointList;
class EntityHitboxList;

class FrameList : public Accessor::NamedListAccessor<SI::Frame, Document> {
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
    bool editSelected_setFrameLocation(SI::FrameLocation& frameLocation);
    bool editSelected_setSolid(bool solid);
    bool editSelected_toggleTileHitbox();

    // Will also edit frame.solid
    bool editSelected_setTileHitbox(const urect& hitbox);

    // ::TODO remove::
protected:
    friend class AbstractFrameContentAccessor;
    SI::Frame* selectedItemEditable();

signals:
    void frameLocationChanged(index_type index);

    void tileHitboxSelectedChanged();
};

class AbstractFrameContentAccessor : public QObject {
    Q_OBJECT

public:
    using index_type = size_t;
    using ArgsT = std::tuple<size_t>;
    using SignalArgsT = std::tuple<size_t>;

protected:
    Document* const _document;

private:
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
        return std::make_tuple(_document->frameList()->selectedIndex());
    }

    inline SI::Frame* getFrame(size_t frameIndex)
    {
        auto* fs = _document->frameSet();
        if (fs == nullptr || frameIndex >= fs->frames.size()) {
            return nullptr;
        }
        return &fs->frames.at(frameIndex);
    }

signals:
    void dataChanged(size_t frameIndex, index_type index);
    void listChanged(size_t frameIndex);

    void listAboutToChange(size_t frameIndex);
    void itemAdded(size_t frameIndex, index_type index);
    void itemAboutToBeRemoved(size_t frameIndex, index_type index);
    void itemMoved(size_t frameIndex, index_type from, index_type to);

    void selectedListChanged();
    void selectedIndexesChanged();
};

class FrameObjectList : public AbstractFrameContentAccessor {
    Q_OBJECT

public:
    using DataT = SI::FrameObject;
    using ListT = std::vector<DataT>;

    constexpr static index_type maxSize() { return UnTech::MetaSprite::MAX_FRAME_OBJECTS; }

    using ObjectSize = UnTech::MetaSprite::ObjectSize;

public:
    FrameObjectList(Document* document);
    ~FrameObjectList() = default;

    static QString typeName() { return tr("Frame Object"); }
    static QString typeNamePlural() { return tr("Frame Objects"); }

    bool editSelectedList_setLocation(unsigned index, const upoint& location);
    bool editSelectedList_setSize(unsigned index, ObjectSize size);

    bool editSelected_toggleObjectSize();

protected:
    friend class Accessor::ListUndoHelper<FrameObjectList>;
    friend class Accessor::ListActionHelper;
    ListT* getList(size_t frameIndex)
    {
        auto* frame = getFrame(frameIndex);
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
    using ListT = std::vector<DataT>;

    constexpr static index_type maxSize() { return UnTech::MetaSprite::MAX_ACTION_POINTS; }

    using ParameterType = UnTech::MetaSprite::ActionPointParameter;

public:
    ActionPointList(Document* document);
    ~ActionPointList() = default;

    static QString typeName() { return tr("Action Point"); }
    static QString typeNamePlural() { return tr("Action Points"); }

    bool editSelectedList_setLocation(unsigned index, const upoint& location);
    bool editSelectedList_setParameter(unsigned index, ParameterType parameter);

protected:
    friend class Accessor::ListUndoHelper<ActionPointList>;
    friend class Accessor::ListActionHelper;
    ListT* getList(size_t frameIndex)
    {
        auto* frame = getFrame(frameIndex);
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
    using ListT = std::vector<DataT>;

    constexpr static index_type maxSize() { return UnTech::MetaSprite::MAX_ENTITY_HITBOXES; }

    using EntityHitboxType = UnTech::MetaSprite::EntityHitboxType;

public:
    EntityHitboxList(Document* document);
    ~EntityHitboxList() = default;

    static QString typeName() { return tr("Entity Hitbox"); }
    static QString typeNamePlural() { return tr("Entity Hitboxes"); }

    bool editSelectedList_setAabb(unsigned index, const urect& aabb);
    bool editSelectedList_setEntityHitboxType(unsigned index, EntityHitboxType type);

    bool editSelected_setEntityHitboxType(EntityHitboxType type);

protected:
    friend class Accessor::ListUndoHelper<EntityHitboxList>;
    friend class Accessor::ListActionHelper;
    ListT* getList(size_t frameIndex)
    {
        auto* frame = getFrame(frameIndex);
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
