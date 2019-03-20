/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/listundohelper.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::ExportOrder;

ExportNameList::ExportNameList(ExportOrderResourceItem* exportOrder)
    : QObject(exportOrder)
    , _exportOrder(exportOrder)
    , _selectedListIsFrame(true)
    , _selectedIndex(INT_MAX)
{
}

void ExportNameList::setSelectedListIsFrame(bool isFrame)
{
    if (_selectedListIsFrame != isFrame) {
        unselectItem();

        _selectedListIsFrame = isFrame;
        emit selectedListChanged();
    }
}

void ExportNameList::setSelectedIndex(ExportNameList::index_type index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
}

void ExportNameList::setSelectedIndex(bool isFrame, ExportNameList::index_type index)
{
    if (_selectedListIsFrame != isFrame) {
        _selectedListIsFrame = isFrame;
        emit selectedListChanged();
    }
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedListChanged();
    }
}

bool ExportNameList::isSelectedItemValid() const
{
    auto* eo = _exportOrder->exportOrderEditable();
    if (eo == nullptr) {
        return false;
    }
    const auto& nl = _selectedListIsFrame ? &eo->stillFrames : &eo->animations;
    return _selectedIndex < nl->size();
}

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

AlternativesList::AlternativesList(ExportOrderResourceItem* exportOrder)
    : QObject(exportOrder)
    , _exportOrder(exportOrder)
    , _selectedIndex(INT_MAX)
{
    connect(_exportOrder->exportNameList(), &ExportNameList::selectedIndexChanged,
            this, &AlternativesList::unselectItem);
}

void AlternativesList::setSelectedIndex(AlternativesList::index_type index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
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
