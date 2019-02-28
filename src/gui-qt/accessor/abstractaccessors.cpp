/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractaccessors.h"
#include "models/common/namedlist.h"

using namespace UnTech::GuiQt::Accessor;

AbstractListAccessor::AbstractListAccessor(QObject* parent, size_t maxSize)
    : QObject(parent)
    , _maxSize(maxSize)
    , _selectedIndex(INT_MAX)
{
    connect(this, &AbstractListAccessor::dataChanged,
            this, &AbstractListAccessor::onDataChanged);
    connect(this, &AbstractListAccessor::itemAdded,
            this, &AbstractListAccessor::onItemAdded);
    connect(this, &AbstractListAccessor::itemAboutToBeRemoved,
            this, &AbstractListAccessor::onItemAboutToBeRemoved);
    connect(this, &AbstractListAccessor::itemMoved,
            this, &AbstractListAccessor::onItemMoved);
}

void AbstractListAccessor::onDataChanged(size_t index)
{
    if (index == _selectedIndex) {
        emit selectedDataChanged();
    }
}

void AbstractListAccessor::onItemAdded(size_t index)
{
    if (_selectedIndex < size()) {
        if (_selectedIndex >= index) {
            setSelectedIndex(_selectedIndex + 1);
        }
    }
}

void AbstractListAccessor::onItemAboutToBeRemoved(size_t index)
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

void AbstractListAccessor::onItemMoved(size_t from, size_t to)
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

void AbstractListAccessor::setSelectedIndex(size_t index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
}

bool AbstractListAccessor::addItem()
{
    return addItem(size());
}

bool AbstractListAccessor::cloneSelectedItem()
{
    return cloneItem(_selectedIndex);
}

bool AbstractListAccessor::addItem(size_t index)
{
    bool s = do_addItem(index);
    if (s) {
        setSelectedIndex(index);
    }
    return s;
}

bool AbstractListAccessor::cloneItem(size_t index)
{
    bool s = do_cloneItem(index);
    if (s) {
        setSelectedIndex(index + 1);
    }
    return s;
}

bool AbstractListAccessor::removeSelectedItem()
{
    return removeItem(_selectedIndex);
}

bool AbstractListAccessor::raiseSelectedItemToTop()
{
    return moveItem(_selectedIndex, 0);
}

bool AbstractListAccessor::raiseSelectedItem()
{
    if (_selectedIndex == 0) {
        return false;
    }
    return moveItem(_selectedIndex, _selectedIndex - 1);
}

bool AbstractListAccessor::lowerSelectedItem()
{
    return moveItem(_selectedIndex, _selectedIndex + 1);
}

bool AbstractListAccessor::lowerSelectedItemToBottom()
{
    auto s = size();
    if (s == 0) {
        return false;
    }
    return moveItem(_selectedIndex, s - 1);
}

AbstractNamedListAccessor::AbstractNamedListAccessor(QObject* parent, size_t maxSize)
    : AbstractListAccessor(parent, maxSize)
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
