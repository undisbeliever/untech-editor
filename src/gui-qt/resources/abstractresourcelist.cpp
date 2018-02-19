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

    _state = size > 0 ? ResourceState::DIRTY : ResourceState::VALID;
    emit stateChanged();
}

void AbstractResourceList::updateState()
{
    bool error = false;
    bool dirty = false;

    for (const auto& item : _list) {
        ResourceState s = item->state();
        if (s == ResourceState::ERROR) {
            error = true;
        }
        else if (s == ResourceState::DIRTY) {
            dirty = true;
        }
    }

    ResourceState s;
    if (error) {
        s = ResourceState::ERROR;
    }
    else if (dirty) {
        s = ResourceState::DIRTY;
    }
    else {
        s = ResourceState::VALID;
    }

    if (_state != s) {
        _state = s;
        emit stateChanged();
    }
}
