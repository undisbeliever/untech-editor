/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractresourcelist.h"
#include "abstractresourceitem.h"

using namespace UnTech::GuiQt::Resources;

AbstractResourceList::AbstractResourceList(QObject* parent, ResourceTypeIndex typeIndex)
    : QObject(parent)
    , _list()
    , _document(nullptr)
    , _resourceTypeIndex(typeIndex)
    , _state(ResourceState::VALID)
{
}

void AbstractResourceList::setDocument(Document* document)
{
    Q_ASSERT(document != nullptr);
    Q_ASSERT(_list.isEmpty());

    _document = document;

    // build list
    size_t size = nItems();
    _list.reserve(size);

    for (size_t i = 0; i < size; i++) {
        AbstractResourceItem* item = buildResourceItem(i);
        _list.append(item);

        item->connect(item, &AbstractResourceItem::stateChanged,
                      this, &AbstractResourceList::updateState);
    }

    _state = size > 0 ? ResourceState::UNCHECKED : ResourceState::VALID;
    emit stateChanged();
}

void AbstractResourceList::updateState()
{
    bool error = false;
    bool unchecked = false;

    for (const auto& item : _list) {
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
