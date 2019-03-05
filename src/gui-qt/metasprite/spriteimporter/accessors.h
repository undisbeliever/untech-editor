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

class FrameList : public Accessor::NamedListAccessor<SI::Frame, Document> {
    Q_OBJECT

    using SpriteOrderType = UnTech::MetaSprite::SpriteOrderType;

private:
    bool _tileHitboxSelected;

public:
    FrameList(Document* document);
    ~FrameList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

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

class FrameObjectList : public Accessor::ChildVectorMultipleSelectionAccessor<SI::FrameObject, Document> {
    Q_OBJECT

    using ObjectSize = UnTech::MetaSprite::ObjectSize;

public:
    FrameObjectList(Document* document);
    ~FrameObjectList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool editSelectedList_setLocation(unsigned index, const upoint& location);
    bool editSelectedList_setSize(unsigned index, ObjectSize size);

    bool editSelected_toggleObjectSize();
};

class ActionPointList : public Accessor::ChildVectorMultipleSelectionAccessor<SI::ActionPoint, Document> {
    Q_OBJECT

    using ParameterType = UnTech::MetaSprite::ActionPointParameter;

public:
    ActionPointList(Document* document);
    ~ActionPointList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool editSelectedList_setLocation(unsigned index, const upoint& location);
    bool editSelectedList_setParameter(unsigned index, ParameterType parameter);
};

class EntityHitboxList : public Accessor::ChildVectorMultipleSelectionAccessor<SI::EntityHitbox, Document> {
    Q_OBJECT

    using EntityHitboxType = UnTech::MetaSprite::EntityHitboxType;

public:
    EntityHitboxList(Document* document);
    ~EntityHitboxList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool editSelectedList_setAabb(unsigned index, const urect& aabb);
    bool editSelectedList_setEntityHitboxType(unsigned index, EntityHitboxType type);

    bool editSelected_setEntityHitboxType(EntityHitboxType type);
};
}
}
}
}
