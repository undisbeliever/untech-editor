/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractaccessors.h"
#include "gui-qt/abstractresourceitem.h"
#include "models/common/namedlist.h"

using namespace UnTech::GuiQt::Accessor;

AbstractListAccessor::AbstractListAccessor(AbstractResourceItem* resourceItem, size_t maxSize)
    : QObject(resourceItem)
    , _resourceItem(resourceItem)
    , _maxSize(maxSize)
{
}

bool AbstractListAccessor::addItem()
{
    return addItem(INT_MAX);
}

AbstractListSingleSelectionAccessor::AbstractListSingleSelectionAccessor(AbstractResourceItem* resourceItem, size_t maxSize)
    : AbstractListAccessor(resourceItem, maxSize)
    , _selectedIndex(INT_MAX)
{
    connect(this, &AbstractListSingleSelectionAccessor::listReset,
            this, &AbstractListSingleSelectionAccessor::unselectItem);

    connect(this, &AbstractListSingleSelectionAccessor::dataChanged,
            this, &AbstractListSingleSelectionAccessor::onDataChanged);
}

void AbstractListSingleSelectionAccessor::onDataChanged(size_t index)
{
    if (index == _selectedIndex) {
        emit selectedDataChanged();
    }
}

void AbstractListSingleSelectionAccessor::setSelectedIndex(size_t index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
}

bool AbstractListSingleSelectionAccessor::cloneSelectedItem()
{
    return cloneItem(_selectedIndex);
}

bool AbstractListSingleSelectionAccessor::removeSelectedItem()
{
    return removeItem(_selectedIndex);
}

bool AbstractListSingleSelectionAccessor::raiseSelectedItemToTop()
{
    return moveItem(_selectedIndex, 0);
}

bool AbstractListSingleSelectionAccessor::raiseSelectedItem()
{
    if (_selectedIndex == 0) {
        return false;
    }
    return moveItem(_selectedIndex, _selectedIndex - 1);
}

bool AbstractListSingleSelectionAccessor::lowerSelectedItem()
{
    return moveItem(_selectedIndex, _selectedIndex + 1);
}

bool AbstractListSingleSelectionAccessor::lowerSelectedItemToBottom()
{
    return moveItem(_selectedIndex, INT_MAX);
}

AbstractListMultipleSelectionAccessor::AbstractListMultipleSelectionAccessor(AbstractResourceItem* resourceItem, size_t maxSize)
    : AbstractListAccessor(resourceItem, maxSize)
    , _selectedIndexes()
{
    connect(this, &AbstractListMultipleSelectionAccessor::listReset,
            this, &AbstractListMultipleSelectionAccessor::clearSelection);
}

void AbstractListMultipleSelectionAccessor::setSelectedIndexes(const vectorset<size_t>& selected)
{
    if (_selectedIndexes != selected) {
        _selectedIndexes = selected;
        emit selectedIndexesChanged();
    }
}

void AbstractListMultipleSelectionAccessor::setSelectedIndexes(vectorset<size_t>&& selected)
{
    if (_selectedIndexes != selected) {
        _selectedIndexes = std::move(selected);
        emit selectedIndexesChanged();
    }
}

void AbstractListMultipleSelectionAccessor::clearSelection()
{
    if (!_selectedIndexes.empty()) {
        _selectedIndexes.clear();
        emit selectedIndexesChanged();
    }
}

void AbstractListMultipleSelectionAccessor::selectAll()
{
    const size_t size = this->size();

    std::vector<size_t> indexes;
    indexes.reserve(size);
    for (size_t i = 0; i < size; i++) {
        indexes.push_back(i);
    }

    setSelectedIndexes(std::move(indexes));
}

bool AbstractListMultipleSelectionAccessor::cloneSelectedItems()
{
    return cloneMultipleItems(_selectedIndexes);
}

bool AbstractListMultipleSelectionAccessor::removeSelectedItems()
{
    return removeMultipleItems(_selectedIndexes);
}

bool AbstractListMultipleSelectionAccessor::raiseSelectedItemsToTop()
{
    return moveMultipleItems(_selectedIndexes, INT_MIN);
}

bool AbstractListMultipleSelectionAccessor::raiseSelectedItems()
{
    return moveMultipleItems(_selectedIndexes, -1);
}

bool AbstractListMultipleSelectionAccessor::lowerSelectedItems()
{
    return moveMultipleItems(_selectedIndexes, +1);
}

bool AbstractListMultipleSelectionAccessor::lowerSelectedItemsToBottom()
{
    return moveMultipleItems(_selectedIndexes, INT_MAX);
}

AbstractNamedListAccessor::AbstractNamedListAccessor(AbstractResourceItem* resourceItem, size_t maxSize)
    : AbstractListSingleSelectionAccessor(resourceItem, maxSize)
{
}

bool AbstractNamedListAccessor::editSelected_setName(const UnTech::idstring& name)
{
    return edit_setName(selectedIndex(), name);
}

bool AbstractNamedListAccessor::addItemWithName(const UnTech::idstring& name)
{
    return addItemWithName(INT_MAX, name);
}

bool AbstractNamedListAccessor::cloneSelectedItemWithName(const UnTech::idstring& name)
{
    return cloneItemWithName(selectedIndex(), name);
}

AbstractChildListSingleSelectionAccessor::AbstractChildListSingleSelectionAccessor(AbstractListSingleSelectionAccessor* parentAccessor, size_t maxSize)
    : AbstractListSingleSelectionAccessor(parentAccessor->resourceItem(), maxSize)
    , _parentAccessor(parentAccessor)
    , _parentIndex(parentAccessor->selectedIndex())
{
    setParent(_parentAccessor);

    connect(_parentAccessor, &AbstractListSingleSelectionAccessor::listReset,
            this, &AbstractChildListSingleSelectionAccessor::onParentSelectedIndexChanged);
    connect(_parentAccessor, &AbstractListSingleSelectionAccessor::selectedIndexChanged,
            this, &AbstractChildListSingleSelectionAccessor::onParentSelectedIndexChanged);

    connect(this, &AbstractChildListSingleSelectionAccessor::dataChanged,
            this, &AbstractChildListSingleSelectionAccessor::onDataChanged);
    connect(this, &AbstractChildListSingleSelectionAccessor::listChanged,
            this, &AbstractChildListSingleSelectionAccessor::onListChanged);
    connect(this, &AbstractChildListSingleSelectionAccessor::listAboutToChange,
            this, &AbstractChildListSingleSelectionAccessor::onListAboutToChange);
    connect(this, &AbstractChildListSingleSelectionAccessor::itemAdded,
            this, &AbstractChildListSingleSelectionAccessor::onItemAdded);
    connect(this, &AbstractChildListSingleSelectionAccessor::itemAboutToBeRemoved,
            this, &AbstractChildListSingleSelectionAccessor::onItemAboutToBeRemoved);
    connect(this, &AbstractChildListSingleSelectionAccessor::itemMoved,
            this, &AbstractChildListSingleSelectionAccessor::onItemMoved);
}

void AbstractChildListSingleSelectionAccessor::onParentSelectedIndexChanged()
{
    emit AbstractListAccessor::listAboutToChange();
    unselectItem();

    _parentIndex = _parentAccessor->selectedIndex();

    emit AbstractListAccessor::listReset();
    emit AbstractListAccessor::listChanged();
}

void AbstractChildListSingleSelectionAccessor::onDataChanged(size_t parentIndex, size_t index)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::dataChanged(index);
    }
}

void AbstractChildListSingleSelectionAccessor::onListChanged(size_t parentIndex)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::listChanged();
    }
}

void AbstractChildListSingleSelectionAccessor::onListAboutToChange(size_t parentIndex)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::listAboutToChange();
    }
}

void AbstractChildListSingleSelectionAccessor::onItemAdded(size_t parentIndex, size_t index)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::itemAdded(index);
    }
}

void AbstractChildListSingleSelectionAccessor::onItemAboutToBeRemoved(size_t parentIndex, size_t index)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::itemAboutToBeRemoved(index);
    }
}

void AbstractChildListSingleSelectionAccessor::onItemMoved(size_t parentIndex, size_t from, size_t to)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::itemMoved(from, to);
    }
}

AbstractChildListMultipleSelectionAccessor::AbstractChildListMultipleSelectionAccessor(AbstractListSingleSelectionAccessor* parentAccessor, size_t maxSize)
    : AbstractListMultipleSelectionAccessor(parentAccessor->resourceItem(), maxSize)
    , _parentAccessor(parentAccessor)
    , _parentIndex(parentAccessor->selectedIndex())
{
    setParent(_parentAccessor);

    connect(_parentAccessor, &AbstractListSingleSelectionAccessor::listReset,
            this, &AbstractChildListMultipleSelectionAccessor::onParentSelectedIndexChanged);
    connect(_parentAccessor, &AbstractListSingleSelectionAccessor::selectedIndexChanged,
            this, &AbstractChildListMultipleSelectionAccessor::onParentSelectedIndexChanged);

    connect(this, &AbstractChildListMultipleSelectionAccessor::dataChanged,
            this, &AbstractChildListMultipleSelectionAccessor::onDataChanged);
    connect(this, &AbstractChildListMultipleSelectionAccessor::listChanged,
            this, &AbstractChildListMultipleSelectionAccessor::onListChanged);
    connect(this, &AbstractChildListMultipleSelectionAccessor::listAboutToChange,
            this, &AbstractChildListMultipleSelectionAccessor::onListAboutToChange);
    connect(this, &AbstractChildListMultipleSelectionAccessor::itemAdded,
            this, &AbstractChildListMultipleSelectionAccessor::onItemAdded);
    connect(this, &AbstractChildListMultipleSelectionAccessor::itemAboutToBeRemoved,
            this, &AbstractChildListMultipleSelectionAccessor::onItemAboutToBeRemoved);
    connect(this, &AbstractChildListMultipleSelectionAccessor::itemMoved,
            this, &AbstractChildListMultipleSelectionAccessor::onItemMoved);
}

void AbstractChildListMultipleSelectionAccessor::onParentSelectedIndexChanged()
{
    emit AbstractListAccessor::listAboutToChange();
    clearSelection();

    _parentIndex = _parentAccessor->selectedIndex();

    emit AbstractListAccessor::listReset();
    emit AbstractListAccessor::listChanged();
}

void AbstractChildListMultipleSelectionAccessor::onDataChanged(size_t parentIndex, size_t index)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::dataChanged(index);
    }
}

void AbstractChildListMultipleSelectionAccessor::onListChanged(size_t parentIndex)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::listChanged();
    }
}

void AbstractChildListMultipleSelectionAccessor::onListAboutToChange(size_t parentIndex)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::listAboutToChange();
    }
}

void AbstractChildListMultipleSelectionAccessor::onItemAdded(size_t parentIndex, size_t index)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::itemAdded(index);
    }
}

void AbstractChildListMultipleSelectionAccessor::onItemAboutToBeRemoved(size_t parentIndex, size_t index)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::itemAboutToBeRemoved(index);
    }
}

void AbstractChildListMultipleSelectionAccessor::onItemMoved(size_t parentIndex, size_t from, size_t to)
{
    if (parentIndex == _parentIndex) {
        emit AbstractListAccessor::itemMoved(from, to);
    }
}

AbstractGridAccessor::AbstractGridAccessor(AbstractResourceItem* resourceItem)
    : QObject(resourceItem)
    , _resourceItem(resourceItem)
    , _selectedCells()
{
    connect(this, &AbstractGridAccessor::gridReset,
            this, &AbstractGridAccessor::clearSelection);
    connect(this, &AbstractGridAccessor::gridAboutToBeResized,
            this, &AbstractGridAccessor::clearSelection);
}

void AbstractGridAccessor::setSelectedCells(const upoint_vectorset& selection)
{
    if (_selectedCells != selection) {
        if (testSelection(selection) == false) {
            return clearSelection();
        }
        _selectedCells = selection;
        emit selectedCellsChanged();
    }
}

void AbstractGridAccessor::setSelectedCells(upoint_vectorset&& selection)
{
    if (_selectedCells != selection) {
        if (testSelection(selection) == false) {
            return clearSelection();
        }
        _selectedCells = std::move(selection);
        emit selectedCellsChanged();
    }
}

void AbstractGridAccessor::clearSelection()
{
    if (!_selectedCells.empty()) {
        _selectedCells.clear();
        emit selectedCellsChanged();
    }
}

bool AbstractGridAccessor::testSelection(const upoint_vectorset& selection) const
{
    const usize gridSize = this->size();

    for (const upoint& c : selection) {
        if (c.x >= gridSize.width || c.y >= gridSize.height) {
            return false;
        }
    }

    return true;
}
