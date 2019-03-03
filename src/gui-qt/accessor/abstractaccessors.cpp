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
    return addItem(size());
}

AbstractListSingleSelectionAccessor::AbstractListSingleSelectionAccessor(AbstractResourceItem* resourceItem, size_t maxSize)
    : AbstractListAccessor(resourceItem, maxSize)
    , _selectedIndex(INT_MAX)
{
    connect(this, &AbstractListSingleSelectionAccessor::listReset,
            this, &AbstractListSingleSelectionAccessor::unselectItem);

    connect(this, &AbstractListSingleSelectionAccessor::dataChanged,
            this, &AbstractListSingleSelectionAccessor::onDataChanged);
    connect(this, &AbstractListSingleSelectionAccessor::itemAdded,
            this, &AbstractListSingleSelectionAccessor::onItemAdded);
    connect(this, &AbstractListSingleSelectionAccessor::itemAboutToBeRemoved,
            this, &AbstractListSingleSelectionAccessor::onItemAboutToBeRemoved);
    connect(this, &AbstractListSingleSelectionAccessor::itemMoved,
            this, &AbstractListSingleSelectionAccessor::onItemMoved);
}

void AbstractListSingleSelectionAccessor::onDataChanged(size_t index)
{
    if (index == _selectedIndex) {
        emit selectedDataChanged();
    }
}

void AbstractListSingleSelectionAccessor::onItemAdded(size_t index)
{
    if (_selectedIndex < size()) {
        if (_selectedIndex >= index) {
            setSelectedIndex(_selectedIndex + 1);
        }
    }
}

void AbstractListSingleSelectionAccessor::onItemAboutToBeRemoved(size_t index)
{
    if (_selectedIndex < size()) {
        if (_selectedIndex == index) {
            unselectItem();
        }
        else if (_selectedIndex > index) {
            setSelectedIndex(_selectedIndex - 1);
        }
    }
}

void AbstractListSingleSelectionAccessor::onItemMoved(size_t from, size_t to)
{
    if (_selectedIndex < size()) {
        if (_selectedIndex == from) {
            setSelectedIndex(to);
        }
        else if (_selectedIndex > from && _selectedIndex <= to) {
            setSelectedIndex(_selectedIndex - 1);
        }
        else if (_selectedIndex >= to && _selectedIndex < from) {
            setSelectedIndex(_selectedIndex + 1);
        }
    }
}

void AbstractListSingleSelectionAccessor::setSelectedIndex(size_t index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
}

bool AbstractListSingleSelectionAccessor::addItem(size_t index)
{
    bool s = do_addItem(index);
    if (s) {
        setSelectedIndex(index);
    }
    return s;
}

bool AbstractListSingleSelectionAccessor::cloneItem(size_t index)
{
    bool s = do_cloneItem(index);
    if (s) {
        setSelectedIndex(index + 1);
    }
    return s;
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
    auto s = size();
    if (s == 0) {
        return false;
    }
    return moveItem(_selectedIndex, s - 1);
}

AbstractListMultipleSelectionAccessor::AbstractListMultipleSelectionAccessor(AbstractResourceItem* resourceItem, size_t maxSize)
    : AbstractListAccessor(resourceItem, maxSize)
    , _selectedIndexes()
{
    connect(this, &AbstractListMultipleSelectionAccessor::listReset,
            this, &AbstractListMultipleSelectionAccessor::clearSelection);

    connect(this, &AbstractListMultipleSelectionAccessor::itemAdded,
            this, &AbstractListMultipleSelectionAccessor::onItemAdded);
    connect(this, &AbstractListMultipleSelectionAccessor::itemAboutToBeRemoved,
            this, &AbstractListMultipleSelectionAccessor::onItemAboutToBeRemoved);
    connect(this, &AbstractListMultipleSelectionAccessor::itemMoved,
            this, &AbstractListMultipleSelectionAccessor::onItemMoved);
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

bool AbstractListMultipleSelectionAccessor::addItem(size_t index)
{
    bool s = do_addItem(index);
    if (s) {
        setSelectedIndexes({ index });
    }
    return s;
}

bool AbstractListMultipleSelectionAccessor::cloneItem(size_t index)
{
    bool s = do_cloneItem(index);
    if (s) {
        setSelectedIndexes({ index + 1 });
    }
    return s;
}

bool AbstractListMultipleSelectionAccessor::cloneSelectedItems()
{
    bool s = do_cloneMultipleItems(_selectedIndexes);
    if (s) {
        const auto& indexes = _selectedIndexes;
        std::vector<size_t> newIndexes;
        newIndexes.reserve(indexes.size());
        for (auto it = indexes.begin(); it != indexes.end(); it++) {
            newIndexes.push_back(*it + std::distance(indexes.begin(), it) + 1);
        }
        setSelectedIndexes(std::move(newIndexes));
    }
    return s;
}

bool AbstractListMultipleSelectionAccessor::removeSelectedItems()
{
    return removeMultipleItems(_selectedIndexes);
}

bool AbstractListMultipleSelectionAccessor::raiseSelectedItems()
{
    return moveMultipleItems(_selectedIndexes, -1);
}

bool AbstractListMultipleSelectionAccessor::lowerSelectedItems()
{
    return moveMultipleItems(_selectedIndexes, +1);
}

void AbstractListMultipleSelectionAccessor::onItemAdded(size_t index)
{
    std::vector<size_t> newSel;
    newSel.reserve(_selectedIndexes.size());

    for (const size_t& i : _selectedIndexes) {
        if (i >= index) {
            newSel.push_back(i + 1);
        }
        else {
            newSel.push_back(i);
        }
    }

    setSelectedIndexes(std::move(newSel));
}

void AbstractListMultipleSelectionAccessor::onItemAboutToBeRemoved(size_t index)
{
    std::vector<size_t> newSel;
    newSel.reserve(_selectedIndexes.size());

    for (const size_t& i : _selectedIndexes) {
        if (i < index) {
            newSel.push_back(i);
        }
        else if (i > index) {
            newSel.push_back(i - 1);
        }
    }

    setSelectedIndexes(std::move(newSel));
}

void AbstractListMultipleSelectionAccessor::onItemMoved(size_t from, size_t to)
{
    std::vector<size_t> newSel;
    newSel.reserve(_selectedIndexes.size());

    for (const size_t& i : _selectedIndexes) {
        if (i == from) {
            newSel.push_back(to);
        }
        else if (i > from && i <= to) {
            newSel.push_back(i - 1);
        }
        else if (i >= to && i < from) {
            newSel.push_back(i + 1);
        }
        else {
            newSel.push_back(i);
        }
    }

    setSelectedIndexes(std::move(newSel));
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
    return addItemWithName(size(), name);
}

bool AbstractNamedListAccessor::addItemWithName(size_t index, const UnTech::idstring& name)
{
    bool s = do_addItemWithName(index, name);
    if (s) {
        setSelectedIndex(index);
    }
    return s;
}

bool AbstractNamedListAccessor::cloneSelectedItemWithName(const UnTech::idstring& name)
{
    return cloneItemWithName(selectedIndex(), name);
}

bool AbstractNamedListAccessor::cloneItemWithName(size_t index, const UnTech::idstring& name)
{
    bool s = do_cloneItemWithName(index, name);
    if (s) {
        setSelectedIndex(index + 1);
    }
    return s;
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
