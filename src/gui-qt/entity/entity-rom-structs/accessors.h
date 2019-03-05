/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityromstructsresourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/accessor/accessor.h"
#include "gui-qt/project.h"
#include "models/entity/entityromdata.h"
#include "models/project/project.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Entity {

namespace EN = UnTech::Entity;

class EntityRomStructList : public Accessor::NamedListAccessor<EN::EntityRomStruct, EntityRomStructsResourceItem> {
    Q_OBJECT

    using UndoHelper = Accessor::ListAndSelectionUndoHelper<EntityRomStructList>;

public:
    EntityRomStructList(EntityRomStructsResourceItem* resourceItem);
    ~EntityRomStructList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    void editSelected_setParent(const idstring& parent);
    void editSelected_setComment(const std::string& comment);

signals:
    void parentChanged(index_type);
    void commentChanged(index_type);
};

class EntityRomStructFieldList : public Accessor::ChildVectorAccessor<EN::StructField, EntityRomStructsResourceItem> {
    Q_OBJECT

public:
    EntityRomStructFieldList(EntityRomStructList* resourceItem);
    ~EntityRomStructFieldList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool editSelectedList_setName(size_t index, const idstring& name);
    bool editSelectedList_setType(size_t index, EN::DataType type);
    bool editSelectedList_setDefaultValue(size_t index, const std::string& value);
    bool editSelectedList_setComment(size_t index, const std::string& comment);
};

}
}
}
