/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "nestedlistaccessors.h"
#include "listactionhelper.h"
#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/accessor/nestedlistundohelper.h"
#include "models/common/namedlist.h"

using namespace UnTech::GuiQt::Accessor;

AbstractNestedListMultipleSelectionAccessor::AbstractNestedListMultipleSelectionAccessor(AbstractResourceItem* resourceItem, size_t maxChildListSize)
    : QObject(resourceItem)
    , _resourceItem(resourceItem)
    , _maxChildListSize(maxChildListSize)
    , _selectedIndexes()
    , _parentIndex(std::nullopt)
{
    connect(this, &AbstractNestedListMultipleSelectionAccessor::listReset,
            this, &AbstractNestedListMultipleSelectionAccessor::clearSelection);
}

void AbstractNestedListMultipleSelectionAccessor::updateParentIndex()
{
    if (_selectedIndexes.empty()) {
        _parentIndex = std::nullopt;
    }
    else if (_selectedIndexes.front().first != _selectedIndexes.back().first) {
        _parentIndex = std::nullopt;
    }
    else {
        _parentIndex = _selectedIndexes.front().first;
    }
}

void AbstractNestedListMultipleSelectionAccessor::setSelectedIndex(const index_type parentIndex, const index_type childIndex)
{
    const index_pair_t index{ parentIndex, childIndex };

    if (_selectedIndexes.size() != 1 || _selectedIndexes.front() != index) {
        _selectedIndexes.clear();
        _selectedIndexes.insert(index);

        _parentIndex = parentIndex;

        emit selectedIndexesChanged();
    }
}

void AbstractNestedListMultipleSelectionAccessor::setSelectedIndexes(const vectorset<index_pair_t>& selected)
{
    if (_selectedIndexes != selected) {
        _selectedIndexes = selected;
        updateParentIndex();

        emit selectedIndexesChanged();
    }
}

void AbstractNestedListMultipleSelectionAccessor::setSelectedIndexes(vectorset<AbstractNestedListMultipleSelectionAccessor::index_pair_t>&& selected)
{
    if (_selectedIndexes != selected) {
        _selectedIndexes = std::move(selected);
        updateParentIndex();

        emit selectedIndexesChanged();
    }
}

void AbstractNestedListMultipleSelectionAccessor::clearSelection()
{
    if (!_selectedIndexes.empty()) {
        _selectedIndexes.clear();
        _parentIndex = std::nullopt;

        emit selectedIndexesChanged();
    }
}

void AbstractNestedListMultipleSelectionAccessor::selectAll()
{
    std::vector<index_pair_t> indexes;

    const size_t parentListSize = this->parentListSize();
    for (size_t parentIndex = 0; parentIndex < parentListSize; parentIndex++) {
        const size_t childListSize = this->childListSize(parentIndex);
        for (size_t childIndex = 0; childIndex < childListSize; childIndex++) {
            indexes.emplace_back(parentIndex, childIndex);
        }
    }

    setSelectedIndexes(std::move(indexes));
}

ListActionStatus AbstractNestedListMultipleSelectionAccessor::listActionStatus(const size_t parentIndexForAddAction) const
{
    const size_t parentListSize = this->parentListSize();

    const bool canAdd = parentIndexForAddAction < parentListSize && childListSize(parentIndexForAddAction) < maxChildListSize();

    if (_selectedIndexes.empty()) {
        return {
            .selectionValid = false,
            .canAdd = canAdd,
            .canClone = false,
            .canRemove = false,
            .canRaise = false,
            .canLower = false,
        };
    }

    bool selectionValid = true;
    bool canClone = true;

    bool canRemove = false;
    bool canRaise = false;
    bool canLower = false;

    auto it = _selectedIndexes.begin();
    while (it != _selectedIndexes.end()) {
        const index_type parentIndex = it->first;

        const size_t firstChildIndex = it->second;
        index_type count = 0;

        while (it != _selectedIndexes.end() && it->first == parentIndex) {
            count++;
            it++;
        }
        Q_ASSERT(count > 0);
        size_t lastChildIndex = (it - 1)->second;

        const index_type childListSize = this->childListSize(parentIndex);
        if (parentIndex < parentListSize) {
            const bool selValid = lastChildIndex < childListSize;

            selectionValid &= selValid;
            canClone &= selValid && childListSize + count <= maxChildListSize();
            // ::TODO add canRaiseToTop::
            canRaise |= selValid && firstChildIndex > 0;
            canLower |= selValid && lastChildIndex + 1 < childListSize;
            // ::TODO add canLowerToBottom::
            canRemove |= selValid;
        }
    }

    return {
        .selectionValid = selectionValid,
        .canAdd = canAdd,
        .canClone = canClone,
        .canRemove = canRemove,
        .canRaise = canRaise,
        .canLower = canLower,
    };
}

bool AbstractNestedListMultipleSelectionAccessor::cloneSelectedItems()
{
    return cloneMultipleItems(_selectedIndexes);
}

bool AbstractNestedListMultipleSelectionAccessor::removeSelectedItems()
{
    return removeMultipleItems(_selectedIndexes);
}

bool AbstractNestedListMultipleSelectionAccessor::raiseSelectedItemsToTop()
{
    return moveMultipleItems(_selectedIndexes, INT_MIN);
}

bool AbstractNestedListMultipleSelectionAccessor::raiseSelectedItems()
{
    return moveMultipleItems(_selectedIndexes, -1);
}

bool AbstractNestedListMultipleSelectionAccessor::lowerSelectedItems()
{
    return moveMultipleItems(_selectedIndexes, +1);
}

bool AbstractNestedListMultipleSelectionAccessor::lowerSelectedItemsToBottom()
{
    return moveMultipleItems(_selectedIndexes, INT_MAX);
}

bool AbstractNestedListMultipleSelectionAccessor::moveSelectedItemsToChildList(size_t destinationParentIndex)
{
    return moveMultipleItemsToChildList(_selectedIndexes, destinationParentIndex);
}

bool AbstractNestedListMultipleSelectionAccessor::addItem(size_t parentIndex)
{
    return addItem(parentIndex, this->childListSize(parentIndex));
}
