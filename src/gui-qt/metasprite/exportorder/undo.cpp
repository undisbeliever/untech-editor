/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "exportorderaccessors.h"
#include "gui-qt/accessor/listundohelper.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::ExportOrder;

using ExportNameUndoHelper = ListAndSelectionUndoHelper<ExportNameList>;

bool ExportNameList::editList_setName(bool isFrame, ExportNameList::index_type index, const UnTech::idstring& name)
{
    using ExportName = UnTech::MetaSprite::FrameSetExportOrder::ExportName;

    if (name.isValid() == false) {
        return false;
    }

    return ExportNameUndoHelper(this).editField(
        std::make_tuple(isFrame), index, name,
        tr("Edit Export Name"),
        [](ExportName& en) -> idstring& { return en.name; });
}

bool ExportNameList::editList_addFrame()
{
    setSelectedListIsFrame(true);
    return ExportNameUndoHelper(this).addItemToSelectedList();
}

bool ExportNameList::editList_addAnimation()
{
    setSelectedListIsFrame(false);
    return ExportNameUndoHelper(this).addItemToSelectedList();
}

bool ExportNameList::editSelectedList_cloneSelected()
{
    return ExportNameUndoHelper(this).cloneSelectedItem();
}

bool ExportNameList::editSelectedList_removeSelected()
{
    return ExportNameUndoHelper(this).removeSelectedItem();
}

bool ExportNameList::editSelectedList_raiseSelectedToTop()
{
    return ExportNameUndoHelper(this).raiseSelectedItemToTop();
}

bool ExportNameList::editSelectedList_raiseSelected()
{
    return ExportNameUndoHelper(this).raiseSelectedItem();
}

bool ExportNameList::editSelectedList_lowerSelected()
{
    return ExportNameUndoHelper(this).lowerSelectedItem();
}

bool ExportNameList::editSelectedList_lowerSelectedToBottom()
{
    return ExportNameUndoHelper(this).lowerSelectedItemToBottom();
}

using AlternativesUndoHelper = ListAndSelectionUndoHelper<AlternativesList>;

bool AlternativesList::editList_setValue(bool isFrame, index_type exportIndex, index_type altIndex,
                                         const AlternativesList::DataT& value)
{
    return AlternativesUndoHelper(this).edit(
        std::make_tuple(isFrame, exportIndex), altIndex, value);
}

bool AlternativesList::editSelectedList_addItem()
{
    return AlternativesUndoHelper(this).addItemToSelectedList();
}

bool AlternativesList::editSelectedList_cloneSelected()
{
    return AlternativesUndoHelper(this).cloneSelectedItem();
}

bool AlternativesList::editSelectedList_removeSelected()
{
    return AlternativesUndoHelper(this).removeSelectedItem();
}

bool AlternativesList::editSelectedList_raiseSelectedToTop()
{
    return AlternativesUndoHelper(this).raiseSelectedItemToTop();
}

bool AlternativesList::editSelectedList_raiseSelected()
{
    return AlternativesUndoHelper(this).raiseSelectedItem();
}

bool AlternativesList::editSelectedList_lowerSelected()
{
    return AlternativesUndoHelper(this).lowerSelectedItem();
}

bool AlternativesList::editSelectedList_lowerSelectedToBottom()
{
    return AlternativesUndoHelper(this).lowerSelectedItemToBottom();
}
