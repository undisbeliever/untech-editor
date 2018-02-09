/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractresourcelist.h"
#include "abstractresourceitem.h"

using namespace UnTech::GuiQt::Resources;

AbstractResourceList::AbstractResourceList(QObject* parent)
    : QObject(parent)
    , _list()
    , _document(nullptr)
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
        _list.append(buildResourceItem(i));
    }
}
