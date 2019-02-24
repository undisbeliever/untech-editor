/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/staticresourcelist.h"
#include "models/entity/entityromdata.h"

namespace UnTech {
namespace GuiQt {
class StaticResourceList;

namespace Entity {
class EntityFunctionTableList;

class EntityFunctionTablesResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

    using EntityRomData = UnTech::Entity::EntityRomData;
    using StructFieldMap = UnTech::Entity::StructFieldMap;

public:
    EntityFunctionTablesResourceItem(StaticResourceList* list, unsigned index);
    ~EntityFunctionTablesResourceItem() = default;

    StaticResourceList* resourceList() const { return static_cast<StaticResourceList*>(_list); }

    EntityFunctionTableList* functionTableList() const { return _functionTableList; }

protected:
    virtual bool compileResource(ErrorList& err) final;

private:
    EntityFunctionTableList* const _functionTableList;
};

}
}
}
