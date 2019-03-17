/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "listaccessortablemanager.h"
#include "abstractaccessors.h"

using namespace UnTech::GuiQt::Accessor;

ListAccessorTableManager::ListAccessorTableManager(QObject* parent)
    : PropertyTableManager(parent)
    , _accessor(nullptr)
{
}

void ListAccessorTableManager::setAccessor(AbstractListAccessor* accessor)
{
    if (_accessor == accessor) {
        return;
    }

    if (_accessor) {
        _accessor->disconnect(this);
    }
    _accessor = accessor;

    if (_accessor) {
        connect(_accessor, &AbstractListAccessor::listReset,
                this, &ListAccessorTableManager::onListReset);

        connect(_accessor, &AbstractListAccessor::dataChanged,
                this, &ListAccessorTableManager::itemChanged);

        connect(_accessor, &AbstractListAccessor::itemAdded,
                this, &ListAccessorTableManager::itemAdded);
        connect(_accessor, &AbstractListAccessor::itemAboutToBeRemoved,
                this, &ListAccessorTableManager::itemRemoved);
        connect(_accessor, &AbstractListAccessor::itemMoved,
                this, &ListAccessorTableManager::itemMoved);

        connect(_accessor, &AbstractListAccessor::listAboutToChange,
                this, &ListAccessorTableManager::listAboutToChange);
    }

    onListReset();

    emit accessorChanged();
}

void ListAccessorTableManager::onListReset()
{
    setEnabled(_accessor ? _accessor->listExists() : false);
    emit dataChanged();
}

int ListAccessorTableManager::rowCount() const
{
    if (_accessor == nullptr) {
        return 0;
    }
    return _accessor->size();
}

bool ListAccessorTableManager::canInsertItem()
{
    if (_accessor == nullptr) {
        return false;
    }
    return _accessor->listExists()
           && _accessor->size() < _accessor->maxSize();
}

bool ListAccessorTableManager::canCloneItem(int index)
{
    if (_accessor == nullptr
        || index < 0) {
        return false;
    }

    size_t size = _accessor->size();

    return _accessor->listExists()
           && unsigned(index) < size
           && size < _accessor->maxSize();
}

bool ListAccessorTableManager::insertItem(int index)
{
    if (_accessor == nullptr) {
        return false;
    }
    return _accessor->addItem(index);
}

bool ListAccessorTableManager::cloneItem(int index)
{
    if (_accessor == nullptr) {
        return false;
    }
    return _accessor->cloneItem(index);
}

bool ListAccessorTableManager::removeItem(int index)
{
    if (_accessor == nullptr) {
        return false;
    }
    return _accessor->removeItem(index);
}

bool ListAccessorTableManager::moveItem(int from, int to)
{
    if (_accessor == nullptr) {
        return false;
    }
    return _accessor->moveItem(from, to);
}
