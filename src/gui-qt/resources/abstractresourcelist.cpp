/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractresourcelist.h"
#include "abstractresourceitem.h"
#include "document.h"

#include <QMessageBox>

using namespace UnTech::GuiQt::Resources;

AbstractResourceList::AbstractResourceList(QObject* parent, ResourceTypeIndex typeIndex)
    : QObject(parent)
    , _document(nullptr)
    , _resourceTypeIndex(typeIndex)
    , _state(ResourceState::VALID)
    , _items()
{
}

void AbstractResourceList::setDocument(Document* document)
{
    Q_ASSERT(document != nullptr);
    Q_ASSERT(_items.isEmpty());

    _document = document;

    // build list
    size_t size = nItems();
    _items.reserve(size);

    for (size_t i = 0; i < size; i++) {
        appendNewItemToList(i);
    }

    _state = size > 0 ? ResourceState::UNCHECKED : ResourceState::VALID;
    emit stateChanged();
}

void AbstractResourceList::appendNewItemToList(int index)
{
    AbstractResourceItem* item = buildResourceItem(index);
    _items.append(item);

    emit resourceItemCreated(item);
    item->connect(item, &AbstractResourceItem::stateChanged,
                  this, &AbstractResourceList::updateState);
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
    _document->undoStack()->resetClean();

    _document->setSelectedResource(_items.back());
}

void AbstractResourceList::removeResource(int index)
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < _items.size());

    if (_document->selectedResource() == _items.at(index)) {
        _document->setSelectedResource(nullptr);
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
    _document->undoStack()->resetClean();
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
