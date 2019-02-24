/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityromstructsresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "gui-qt/project.h"
#include "models/entity/entityromdata.h"
#include "models/project/project.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Entity {

namespace EN = UnTech::Entity;

class EntityRomStructList : public QObject {
    Q_OBJECT

public:
    using DataT = EN::EntityRomStruct;
    using ListT = NamedList<DataT>;
    using index_type = ListT::size_type;

    constexpr static index_type max_size = 255;

    EntityRomStructList(EntityRomStructsResourceItem* resourceItem);
    ~EntityRomStructList() = default;

    static QString typeName() { return tr("Entity ROM Struct"); }

    EntityRomStructsResourceItem* resourceItem() const { return _resourceItem; }

    index_type selectedIndex() const { return _selectedIndex; }
    void setSelectedIndex(index_type index);
    void unselectItem() { setSelectedIndex(INT_MAX); }

    bool isSelectedIndexValid() const;

    const DataT* selectedStruct() const;

    void editSelected_setName(const idstring& name);
    void editSelected_setParent(const idstring& parent);
    void editSelected_setComment(const std::string& comment);

    const ListT* list() const
    {
        const auto* projectFile = _resourceItem->project()->projectFile();
        Q_ASSERT(projectFile);
        return &projectFile->entityRomData.structs;
    }

protected:
    friend class Accessor::NamedListUndoHelper<EntityRomStructList>;
    ListT* getList()
    {
        auto* projectFile = _resourceItem->project()->projectFile();
        Q_ASSERT(projectFile);
        return &projectFile->entityRomData.structs;
    }

signals:
    void nameChanged(index_type index);
    void parentChanged(index_type);
    void commentChanged(index_type);

    void dataChanged(index_type index);
    void listChanged();

    void listAboutToChange();
    void itemAdded(index_type index);
    void itemAboutToBeRemoved(index_type index);
    void itemMoved(index_type from, index_type to);

    void selectedIndexChanged();

private:
    EntityRomStructsResourceItem* const _resourceItem;
    index_type _selectedIndex;
};

class EntityRomStructFieldList : public QObject {
    Q_OBJECT

public:
    using DataT = EN::StructField;
    using ListT = std::vector<DataT>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<size_t>;
    using SignalArgsT = ArgsT;

    constexpr static index_type max_size = 255;

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
