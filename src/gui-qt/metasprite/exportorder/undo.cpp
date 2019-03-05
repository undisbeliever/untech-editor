/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "exportorderaccessors.h"
#include "gui-qt/accessor/listundohelper.h"

using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::ExportOrder;

bool ExportNameList::editList_setName(bool isFrame, ExportNameList::index_type index, const UnTech::idstring& name)
{
    using ExportName = UnTech::MetaSprite::FrameSetExportOrder::ExportName;

    if (name.isValid() == false) {
        return false;
    }

    setSelectedListIsFrame(isFrame);
    return UndoHelper(this).editField(
        index, name,
        tr("Edit Export Name"),
        [](ExportName& en) -> idstring& { return en.name; });
}

bool ExportNameList::editList_addFrame()
{
    setSelectedListIsFrame(true);
    return UndoHelper(this).addItem();
}

bool ExportNameList::editList_addAnimation()
{
    setSelectedListIsFrame(false);
    return UndoHelper(this).addItem();
}

bool ExportNameList::editSelectedList_cloneSelected()
{
    return UndoHelper(this).cloneSelectedItem();
}

bool ExportNameList::editSelectedList_removeSelected()
{
    return UndoHelper(this).removeSelectedItem();
}

bool ExportNameList::editSelectedList_raiseSelectedToTop()
{
    return UndoHelper(this).raiseSelectedItemToTop();
}

bool ExportNameList::editSelectedList_raiseSelected()
{
    return UndoHelper(this).raiseSelectedItem();
}

bool ExportNameList::editSelectedList_lowerSelected()
{
    return UndoHelper(this).lowerSelectedItem();
}

bool ExportNameList::editSelectedList_lowerSelectedToBottom()
{
    return UndoHelper(this).lowerSelectedItemToBottom();
}

bool AlternativesList::editList_setValue(bool isFrame, index_type exportIndex, index_type altIndex,
                                         const AlternativesList::DataT& value)
{
    resourceItem()->exportNameList()->setSelectedIndex(isFrame, exportIndex);
    return UndoHelper(this).editItem(altIndex, value);
}

bool AlternativesList::editSelectedList_addItem()
{
    return UndoHelper(this).addItem();
}

bool AlternativesList::editSelectedList_cloneSelected()
{
    return UndoHelper(this).cloneSelectedItem();
}

bool AlternativesList::editSelectedList_removeSelected()
{
    return UndoHelper(this).removeSelectedItem();
}

bool AlternativesList::editSelectedList_raiseSelectedToTop()
{
    return UndoHelper(this).raiseSelectedItemToTop();
}

bool AlternativesList::editSelectedList_raiseSelected()
{
    return UndoHelper(this).raiseSelectedItem();
}

bool AlternativesList::editSelectedList_lowerSelected()
{
    return UndoHelper(this).lowerSelectedItem();
}

bool AlternativesList::editSelectedList_lowerSelectedToBottom()
{
    return UndoHelper(this).lowerSelectedItemToBottom();
}
