/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractresourcelist.h"
#include "models/resources/error-list.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Resources {
class Document;
class AbstractResourceList;

namespace RES = UnTech::Resources;

class AbstractResourceItem : public QObject {
    Q_OBJECT

public:
    AbstractResourceItem(AbstractResourceList* parent, unsigned index)
        : QObject(parent)
        , _list(parent)
        , _document(parent->document())
        , _index(index)
        , _state(ResourceState::NOT_LOADED)
    {
        Q_ASSERT(parent != nullptr);
        Q_ASSERT(_document != nullptr);
    }

    ~AbstractResourceItem() = default;

    Document* document() const { return _document; }
    inline int index() const { return _index; }

    const ResourceState& state() const { return _state; }
    const RES::ErrorList& errorList() const { return _errorList; }

    ResourceTypeIndex resourceTypeIndex() const { return _list->resourceTypeIndex(); }

    // name of the resource item
    virtual const QString name() const = 0;

    // empty if the resource is inline with the resource document
    virtual const QString filename() const = 0;

    void markUnchecked();

    void validateItem();

public slots:
    void loadResource();

protected:
    virtual bool loadResourceData(RES::ErrorList& err) = 0;

    // compiles the resource to test if valid
    virtual bool compileResource(RES::ErrorList& err) = 0;

private:
    void setState(ResourceState state);

signals:
    void stateChanged();
    void errorListChanged();

protected:
    AbstractResourceList* const _list;
    Document* const _document;

private:
    unsigned _index;

    ResourceState _state;
    RES::ErrorList _errorList;
};
}
}
}
