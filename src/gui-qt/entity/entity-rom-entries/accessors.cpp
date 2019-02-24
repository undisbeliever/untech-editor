/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/selectedindexhelper.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Entity;

EntityRomEntriesList::EntityRomEntriesList(EntityRomEntriesResourceItem* resourceItem,
                                           bool entityList)
    : QObject(resourceItem)
    , _resourceItem(resourceItem)
    , _entityList(entityList)
    , _selectedIndex(INT_MAX)
{
    SelectedIndexHelper::buildAndConnectSlots_NamedList(this);
}

void EntityRomEntriesList::setSelectedIndex(EntityRomEntriesList::index_type index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
}

bool EntityRomEntriesList::isSelectedIndexValid() const
{
    return _selectedIndex < list()->size();
}

const EntityRomEntriesList::DataT* EntityRomEntriesList::selectedEntry() const
{
    const ListT* entries = list();
    if (_selectedIndex >= entries->size()) {
        return nullptr;
    }
    return &entries->at(_selectedIndex);
}
