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

    friend class Accessor::NamedListUndoHelper<EntityRomStructList>;

public:
    EntityRomStructList(EntityRomStructsResourceItem* resourceItem);
    ~EntityRomStructList() = default;

    virtual QString typeName() const final;

    void editSelected_setParent(const idstring& parent);
    void editSelected_setComment(const std::string& comment);

signals:
    void parentChanged(index_type);
    void commentChanged(index_type);
};

class EntityRomStructFieldList : public QObject {
    Q_OBJECT

public:
    using DataT = EN::StructField;
    using ListT = std::vector<DataT>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<size_t>;
    using SignalArgsT = ArgsT;

    constexpr static index_type maxSize() { return 255; }

    EntityRomStructFieldList(EntityRomStructsResourceItem* resourceItem);
    ~EntityRomStructFieldList() = default;

    static QString typeName() { return tr("Struct Field"); }

    EntityRomStructsResourceItem* resourceItem() const { return _resourceItem; }

    index_type selectedIndex() const { return _selectedIndex; }
    void setSelectedIndex(index_type index);
    void unselectItem() { setSelectedIndex(INT_MAX); }

    bool editSelectedList_setName(size_t index, const idstring& name);
    bool editSelectedList_setType(size_t index, EN::DataType type);
    bool editSelectedList_setDefaultValue(size_t index, const std::string& value);
    bool editSelectedList_setComment(size_t index, const std::string& comment);

protected:
    friend class Accessor::ListUndoHelper<EntityRomStructFieldList>;
    friend class Accessor::SelectedIndexHelper;
    friend class Accessor::ListActionHelper;
    ListT* getList(size_t structId)
    {
        auto* projectFile = _resourceItem->project()->projectFile();
        Q_ASSERT(projectFile);
        auto& structs = projectFile->entityRomData.structs;
        if (structId >= structs.size()) {
            return nullptr;
        }
        return &structs.at(structId).fields;
    }

    ArgsT selectedListTuple() const
    {
        return std::make_tuple(_resourceItem->structList()->selectedIndex());
    }

signals:
    void selectedListChanged();

    void dataChanged(size_t structId, index_type index);
    void listChanged(size_t structId);

    void listAboutToChange(size_t structId);
    void itemAdded(size_t structId, index_type index);
    void itemAboutToBeRemoved(size_t structId, index_type index);
    void itemMoved(size_t structId, index_type from, index_type to);

    void selectedIndexChanged();

private:
    EntityRomStructsResourceItem* const _resourceItem;
    index_type _selectedIndex;
};

}
}
}
