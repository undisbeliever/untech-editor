/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractresourcelist.h"
#include "abstractresourceitem.h"
#include "resourceproject.h"

#include <QMessageBox>

using namespace UnTech::GuiQt::Resources;

AbstractResourceList::AbstractResourceList(ResourceProject* project, ResourceTypeIndex typeIndex)
    : QObject(project)
    , _project(project)
    , _resourceTypeIndex(typeIndex)
    , _state(ResourceState::VALID)
    , _items()
{
    Q_ASSERT(_project != nullptr);
}

void AbstractResourceList::rebuildResourceItems()
{
    qDeleteAll(_items);
    _items.clear();

    size_t size = nItems();
    _items.reserve(size);

    for (size_t i = 0; i < size; i++) {
        appendNewItemToList(i);
    }

    _state = size > 0 ? ResourceState::UNCHECKED : ResourceState::VALID;
    emit stateChanged();
    emit listChanged();
}

void AbstractResourceList::appendNewItemToList(int index)
{
    AbstractResourceItem* item = buildResourceItem(index);
    _items.append(item);

    emit resourceItemCreated(item);
    item->connect(item, &AbstractResourceItem::stateChanged,
                  this, &AbstractResourceList::updateState);
}

QStringList AbstractResourceList::itemNames() const
{
    QStringList list;
    list.reserve(_items.size());
    std::transform(_items.begin(), _items.end(), std::back_inserter(list),
                   [](const auto* i) -> const QString& { return i->name(); });
    return list;
}

AbstractResourceItem* AbstractResourceList::findResource(const QString& name) const
{
    for (AbstractResourceItem* item : _items) {
        if (item->name() == name) {
            return item;
        }
    }

    return nullptr;
}

void AbstractResourceList::addResource(const QString& input)
{
    try {
        do_addResource(input.toStdString());
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error Creating Resource"), ex.what());
        return;
    }

    Q_ASSERT((size_t)_items.size() + 1 == nItems());
    appendNewItemToList(_items.size());

    emit listChanged();
    _project->undoStack()->resetClean();

    _project->setSelectedResource(_items.back());
}

void AbstractResourceList::removeResource(int index)
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < _items.size());

    if (_project->selectedResource() == _items.at(index)) {
        _project->setSelectedResource(nullptr);
    }

    emit resourceItemAboutToBeRemoved(_items.at(index));

    do_removeResource(index);

    AbstractResourceItem* oldItem = _items.takeAt(index);
    oldItem->deleteLater();

    for (int i = index; i < _items.size(); i++) {
        _items.at(i)->setIndex(i);
    }

    updateState();

    emit listChanged();
    _project->undoStack()->resetClean();
}

void AbstractResourceList::updateState()
{
    bool error = false;
    bool unchecked = false;

    for (const auto& item : _items) {
        switch (item->state()) {
        case ResourceState::ERROR:
        case ResourceState::FILE_ERROR:
            error = true;
            break;

        case ResourceState::UNCHECKED:
        case ResourceState::NOT_LOADED:
            unchecked = true;
            break;

        case ResourceState::VALID:
            break;
        }
    }

    ResourceState s;
    if (error) {
        s = ResourceState::ERROR;
    }
    else if (unchecked) {
        s = ResourceState::UNCHECKED;
    }
    else {
        s = ResourceState::VALID;
    }

    if (_state != s) {
        _state = s;
        emit stateChanged();
    }
}
