/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractresourcelist.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Resources {
class Document;
class AbstractResourceList;

class AbstractResourceItem : public QObject {
    Q_OBJECT

public:
    AbstractResourceItem(AbstractResourceList* parent, unsigned index)
        : QObject(parent)
        , _list(parent)
        , _document(parent->document())
        , _index(index)
    {
        Q_ASSERT(parent != nullptr);
        Q_ASSERT(_document != nullptr);
    }

    ~AbstractResourceItem() = default;

    Document* document() const { return _document; }
    int index() const { return _index; }

    ResourceTypeIndex resourceTypeIndex() const { return _list->resourceTypeIndex(); }

    // name of the resource item
    virtual const QString name() const = 0;

    // empty if the resource is inline with the resource document
    virtual const QString filename() const = 0;

protected:
    AbstractResourceList* const _list;
    Document* const _document;
    unsigned _index;
};
}
}
}
