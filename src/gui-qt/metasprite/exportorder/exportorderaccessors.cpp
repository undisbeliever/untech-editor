/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "exportorderaccessors.h"
#include "gui-qt/undo/selectedindexhelper.h"

using namespace UnTech::GuiQt::Undo;
using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::ExportOrder;

ExportNameList::ExportNameList(ExportOrderResourceItem* exportOrder)
    : QObject(exportOrder)
    , _exportOrder(exportOrder)
    , _selectedListIsFrame(true)
    , _selectedIndex(INT_MAX)
{
    SelectedIndexHelper::buildAndConnectSlots(this);
}

AlternativesList::AlternativesList(ExportOrderResourceItem* exportOrder)
    : QObject(exportOrder)
    , _exportOrder(exportOrder)
    , _selectedIndex(INT_MAX)
{
    connect(_exportOrder->exportNameList(), &ExportNameList::selectedIndexChanged,
            this, &AlternativesList::unselectItem);

    SelectedIndexHelper::buildAndConnectSlots(this);
}
