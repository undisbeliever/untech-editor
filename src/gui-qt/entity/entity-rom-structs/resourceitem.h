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
namespace EntityRomStructs {
class EntityRomStructList;
class EntityRomStructFieldList;

class ResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

    using EntityRomData = UnTech::Entity::EntityRomData;
    using StructFieldMap = UnTech::Entity::StructFieldMap;

public:
    ResourceItem(StaticResourceList* list, unsigned index);
    ~ResourceItem() = default;

    StaticResourceList* resourceList() const { return static_cast<StaticResourceList*>(_list); }

    EntityRomStructList* structList() const { return _structList; }
    EntityRomStructFieldList* structFieldList() const { return _structFieldList; }

    const StructFieldMap& structFieldMap() const { return _structFieldMap; }

    bool editListIds_setList(const std::vector<idstring>& listIds);
    bool editListIds_setListId(int index, const idstring& value);

protected:
    virtual bool compileResource(ErrorList& err) final;

private:
    EntityRomStructList* _structList;
    EntityRomStructFieldList* _structFieldList;

    StructFieldMap _structFieldMap;
};

}
}
}
}
