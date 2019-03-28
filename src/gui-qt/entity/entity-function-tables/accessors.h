/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityfunctiontablesresourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/project.h"
#include "models/entity/entityromdata.h"
#include "models/project/project.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Entity {
namespace EntityFunctionTables {
class EntityFunctionTablesResourceItem;

namespace EN = UnTech::Entity;

class EntityFunctionTableList : public Accessor::NamedListAccessor<EN::EntityFunctionTable, EntityFunctionTablesResourceItem> {
    Q_OBJECT

public:
    EntityFunctionTableList(EntityFunctionTablesResourceItem* resourceItem);
    ~EntityFunctionTableList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool edit_setEntityStruct(index_type index, const idstring& entityStruct);
    bool edit_setExportOrder(index_type index, const idstring& exportOrder);
    bool edit_setParameterType(index_type index, EN::ParameterType parameterType);
    bool edit_setComment(index_type index, const std::string& comment);
};

}
}
}
}
