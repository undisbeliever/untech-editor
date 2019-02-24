/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityfunctiontablesresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "gui-qt/project.h"
#include "models/entity/entityromdata.h"
#include "models/project/project.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Entity {
class EntityFunctionTablesResourceItem;

namespace EN = UnTech::Entity;

class EntityFunctionTableList : public QObject {
    Q_OBJECT

public:
    using DataT = EN::EntityFunctionTable;
    using ListT = NamedList<DataT>;
    using index_type = ListT::size_type;

    constexpr static index_type max_size = 255;

    EntityFunctionTableList(EntityFunctionTablesResourceItem* resourceItem)
        : QObject(resourceItem)
        , _resourceItem(resourceItem)
    {
    }
    ~EntityFunctionTableList() = default;

    static QString typeName() { return tr("Entity Function Table"); }

    EntityFunctionTablesResourceItem* resourceItem() const { return _resourceItem; }

    bool edit_setName(index_type index, const idstring& name);
    bool edit_setEntityStruct(index_type index, const idstring& entityStruct);
    bool edit_setExportOrder(index_type index, const idstring& exportOrder);
    bool edit_setParameterType(index_type index, EN::ParameterType parameterType);
    bool edit_setComment(index_type index, const std::string& comment);

    const ListT* list() const
    {
        const auto* projectFile = _resourceItem->project()->projectFile();
        Q_ASSERT(projectFile);
        return &projectFile->entityRomData.functionTables;
    }

    const ListT& functionTables() const { return *list(); }

protected:
    friend class Accessor::NamedListUndoHelper<EntityFunctionTableList>;
    ListT* getList()
    {
        auto* projectFile = _resourceItem->project()->projectFile();
        Q_ASSERT(projectFile);
        return &projectFile->entityRomData.functionTables;
    }

signals:
    void nameChanged(index_type index);

    void dataChanged(index_type index);
    void listChanged();

    void listAboutToChange();
    void itemAdded(index_type index);
    void itemAboutToBeRemoved(index_type index);
    void itemMoved(index_type from, index_type to);

    void selectedIndexChanged();

private:
    EntityFunctionTablesResourceItem* const _resourceItem;
};

}
}
}
