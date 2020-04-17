/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/listundohelper.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::ExportOrder;

ExportNameList::ExportNameList(ResourceItem* exportOrder)
    : AbstractListSingleSelectionAccessor(exportOrder, UnTech::MetaSprite::MAX_EXPORT_NAMES)
    , _selectedListIsFrame(true)
{

    connect(this, &ExportNameList::dataChanged,
            this, &ExportNameList::onDataChanged);
    connect(this, &ExportNameList::listChanged,
            this, &ExportNameList::onListChanged);
    connect(this, &ExportNameList::listAboutToChange,
            this, &ExportNameList::onListAboutToChange);
    connect(this, &ExportNameList::itemAdded,
            this, &ExportNameList::onItemAdded);
    connect(this, &ExportNameList::itemAboutToBeRemoved,
            this, &ExportNameList::onItemAboutToBeRemoved);
    connect(this, &ExportNameList::itemMoved,
            this, &ExportNameList::onItemMoved);
}

QString ExportNameList::typeName() const
{
    return tr("Export Name");
}

QString ExportNameList::typeNamePlural() const
{
    return tr("Export Names");
}

void ExportNameList::setSelectedListIsFrame(bool isFrame)
{
    if (_selectedListIsFrame != isFrame) {
        unselectItem();

        _selectedListIsFrame = isFrame;
        emit listReset();
    }
}

void ExportNameList::setSelectedIndex(bool isFrame, index_type index)
{
    if (_selectedListIsFrame != isFrame) {
        _selectedListIsFrame = isFrame;
        emit listReset();
    }
    AbstractListSingleSelectionAccessor::setSelectedIndex(index);
}

ExportNameList::ListT* ExportNameList::getList(bool isFrame)
{
    auto* eo = resourceItem()->dataEditable();
    if (eo == nullptr) {
        return nullptr;
    }
    return isFrame ? &eo->stillFrames : &eo->animations;
}

ExportNameList::ArgsT ExportNameList::selectedListTuple() const
{
    return std::make_tuple(_selectedListIsFrame);
}

const ExportNameList::ListT* ExportNameList::selectedList() const
{
    auto* eo = resourceItem()->dataEditable();
    if (eo == nullptr) {
        return nullptr;
    }
    return _selectedListIsFrame ? &eo->stillFrames : &eo->animations;
}

bool ExportNameList::listExists() const
{
    return selectedList() != nullptr;
}

size_t ExportNameList::size() const
{
    if (auto* l = selectedList()) {
        return l->size();
    }
    return 0;
}

bool ExportNameList::addItem(size_t index)
{
    return UndoHelper(this).addItem(index);
}

bool ExportNameList::cloneItem(size_t index)
{
    return UndoHelper(this).cloneItem(index);
}

bool ExportNameList::removeItem(size_t index)
{
    return UndoHelper(this).removeItem(index);
}

bool ExportNameList::moveItem(size_t from, size_t to)
{
    return UndoHelper(this).moveItem(from, to);
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

void ExportNameList::onDataChanged(bool isFrame, size_t index)
{
    if (isFrame == _selectedListIsFrame) {
        emit AbstractListAccessor::dataChanged(index);
    }
}

void ExportNameList::onListChanged(bool isFrame)
{
    if (isFrame == _selectedListIsFrame) {
        emit AbstractListAccessor::listChanged();
    }
}

void ExportNameList::onListAboutToChange(bool isFrame)
{
    if (isFrame == _selectedListIsFrame) {
        emit AbstractListAccessor::listAboutToChange();
    }
}

void ExportNameList::onItemAdded(bool isFrame, size_t index)
{
    if (isFrame == _selectedListIsFrame) {
        emit AbstractListAccessor::itemAdded(index);
    }
}

void ExportNameList::onItemAboutToBeRemoved(bool isFrame, size_t index)
{
    if (isFrame == _selectedListIsFrame) {
        emit AbstractListAccessor::itemAboutToBeRemoved(index);
    }
}

void ExportNameList::onItemMoved(bool isFrame, size_t from, size_t to)
{
    if (isFrame == _selectedListIsFrame) {
        emit AbstractListAccessor::itemMoved(from, to);
    }
}

AlternativesList::AlternativesList(ResourceItem* exportOrder)
    : AbstractListSingleSelectionAccessor(exportOrder, INT8_MAX)
{
    connect(exportOrder->exportNameList(), &ExportNameList::selectedIndexChanged,
            this, &AlternativesList::onParentSelectedIndexChanged);
    connect(exportOrder->exportNameList(), &ExportNameList::listReset,
            this, &AlternativesList::onParentSelectedIndexChanged);

    connect(this, &AlternativesList::dataChanged,
            this, &AlternativesList::onDataChanged);
    connect(this, &AlternativesList::listChanged,
            this, &AlternativesList::onListChanged);
    connect(this, &AlternativesList::listAboutToChange,
            this, &AlternativesList::onListAboutToChange);
    connect(this, &AlternativesList::itemAdded,
            this, &AlternativesList::onItemAdded);
    connect(this, &AlternativesList::itemAboutToBeRemoved,
            this, &AlternativesList::onItemAboutToBeRemoved);
    connect(this, &AlternativesList::itemMoved,
            this, &AlternativesList::onItemMoved);
}

QString AlternativesList::typeName() const
{
    return tr("Export Name Alternative");
}

QString AlternativesList::typeNamePlural() const
{
    return tr("Export Name Alternatives");
}

AlternativesList::ListT* AlternativesList::getList(bool isFrame, AlternativesList::index_type enIndex)
{
    auto* eo = resourceItem()->dataEditable();
    if (eo == nullptr) {
        return nullptr;
    }

    auto& nameList = isFrame ? eo->stillFrames : eo->animations;

    if (enIndex < nameList.size()) {
        return &nameList.at(enIndex).alternatives;
    }
    else {
        return nullptr;
    }
}

AlternativesList::ArgsT AlternativesList::selectedListTuple() const
{
    const ExportNameList* enl = resourceItem()->exportNameList();
    return std::make_tuple(enl->selectedListIsFrame(), enl->selectedIndex());
}

const AlternativesList::ListT* AlternativesList::selectedList() const
{
    auto [isFrame, enIndex] = selectedListTuple();
    auto* eo = resourceItem()->data();
    if (eo == nullptr) {
        return nullptr;
    }

    auto& nameList = isFrame ? eo->stillFrames : eo->animations;

    if (enIndex < nameList.size()) {
        return &nameList.at(enIndex).alternatives;
    }
    else {
        return nullptr;
    }
}

bool AlternativesList::listExists() const
{
    return selectedList() != nullptr;
}

size_t AlternativesList::size() const
{
    if (auto* l = selectedList()) {
        return l->size();
    }
    return 0;
}

bool AlternativesList::addItem(size_t index)
{
    return UndoHelper(this).addItem(index);
}

bool AlternativesList::cloneItem(size_t index)
{
    return UndoHelper(this).cloneItem(index);
}

bool AlternativesList::removeItem(size_t index)
{
    return UndoHelper(this).removeItem(index);
}

bool AlternativesList::moveItem(size_t from, size_t to)
{
    return UndoHelper(this).moveItem(from, to);
}

bool AlternativesList::editList_setValue(bool isFrame, index_type exportIndex, index_type altIndex,
                                         const AlternativesList::DataT& value)
{
    resourceItem()->exportNameList()->setSelectedIndex(isFrame, exportIndex);
    return UndoHelper(this).editItem(altIndex, value);
}

void AlternativesList::onParentSelectedIndexChanged()
{
    emit AbstractListAccessor::listAboutToChange();
    unselectItem();

    emit AbstractListAccessor::listReset();
    emit AbstractListAccessor::listChanged();
}

void AlternativesList::onDataChanged(bool isFrame, size_t parentIndex, size_t index)
{
    const auto [sl_isFrame, sl_parentIndex] = selectedListTuple();
    if (isFrame == sl_isFrame && parentIndex == sl_parentIndex) {
        emit AbstractListAccessor::dataChanged(index);
    }
}

void AlternativesList::onListChanged(bool isFrame, size_t parentIndex)
{
    const auto [sl_isFrame, sl_parentIndex] = selectedListTuple();
    if (isFrame == sl_isFrame && parentIndex == sl_parentIndex) {
        emit AbstractListAccessor::listChanged();
    }
}

void AlternativesList::onListAboutToChange(bool isFrame, size_t parentIndex)
{
    const auto [sl_isFrame, sl_parentIndex] = selectedListTuple();
    if (isFrame == sl_isFrame && parentIndex == sl_parentIndex) {
        emit AbstractListAccessor::listAboutToChange();
    }
}

void AlternativesList::onItemAdded(bool isFrame, size_t parentIndex, size_t index)
{
    const auto [sl_isFrame, sl_parentIndex] = selectedListTuple();
    if (isFrame == sl_isFrame && parentIndex == sl_parentIndex) {
        emit AbstractListAccessor::itemAdded(index);
    }
}

void AlternativesList::onItemAboutToBeRemoved(bool isFrame, size_t parentIndex, size_t index)
{
    const auto [sl_isFrame, sl_parentIndex] = selectedListTuple();
    if (isFrame == sl_isFrame && parentIndex == sl_parentIndex) {
        emit AbstractListAccessor::itemAboutToBeRemoved(index);
    }
}

void AlternativesList::onItemMoved(bool isFrame, size_t parentIndex, size_t from, size_t to)
{
    const auto [sl_isFrame, sl_parentIndex] = selectedListTuple();
    if (isFrame == sl_isFrame && parentIndex == sl_parentIndex) {
        emit AbstractListAccessor::itemMoved(from, to);
    }
}
