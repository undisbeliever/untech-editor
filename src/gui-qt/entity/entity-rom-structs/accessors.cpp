/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/selectedindexhelper.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Entity;

EntityRomStructList::EntityRomStructList(EntityRomStructsResourceItem* resourceItem)
    : QObject(resourceItem)
    , _resourceItem(resourceItem)
    , _selectedIndex(INT_MAX)
{
    SelectedIndexHelper::buildAndConnectSlots_NamedList(this);
}

void EntityRomStructList::setSelectedIndex(EntityRomStructList::index_type index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
}

bool EntityRomStructList::isSelectedIndexValid() const
{
    return _selectedIndex < list()->size();
}

const EntityRomStructList::DataT* EntityRomStructList::selectedStruct() const
{
    auto& structs = *this->list();
    if (_selectedIndex >= structs.size()) {
        return nullptr;
    }
    return &structs.at(_selectedIndex);
}

EntityRomStructFieldList::EntityRomStructFieldList(EntityRomStructsResourceItem* resourceItem)
    : QObject(resourceItem)
    , _resourceItem(resourceItem)
{
    connect(resourceItem->structList(), &EntityRomStructList::selectedIndexChanged,
            this, &EntityRomStructFieldList::selectedListChanged);
    connect(resourceItem->structList(), &EntityRomStructList::selectedIndexChanged,
            this, &EntityRomStructFieldList::unselectItem);

    SelectedIndexHelper::buildAndConnectSlots(this);
}

void EntityRomStructFieldList::setSelectedIndex(size_t index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
}
