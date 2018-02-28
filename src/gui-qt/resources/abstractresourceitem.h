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
    friend class AbstractInternalResourceItem;

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
    AbstractResourceList* resourceList() const { return _list; }

    // name of the resource item
    virtual QString name() const = 0;

    // empty if the resource is inline with the resource document
    virtual QString filename() const = 0;

    void markUnchecked();

    void validateItem();

public slots:
    void loadResource();

protected:
    virtual bool loadResourceData(RES::ErrorList& err) = 0;

    // compiles the resource to test if valid
    virtual bool compileResource(RES::ErrorList& err) = 0;

private:
    friend void AbstractResourceList::removeResource(int);
    void setIndex(unsigned index);

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

class AbstractInternalResourceItem : public AbstractResourceItem {
    Q_OBJECT

public:
    AbstractInternalResourceItem(AbstractResourceList* parent, unsigned index);

    virtual QString filename() const final;

protected:
    virtual bool loadResourceData(RES::ErrorList&) final;
};

class AbstractExternalResourceItem : public AbstractResourceItem {
    Q_OBJECT

public:
    AbstractExternalResourceItem(AbstractResourceList* parent, unsigned index,
                                 const QString& filename = QString());

    virtual QString filename() const final;

    const QString& absoluteFilePath() const { return _absoluteFilePath; }
    const QString& relativeFilePath() const { return _relativeFilePath; }

protected:
    void setFilename(const QString& filename);

private slots:
    void updateRelativePath();

signals:
    void absoluteFilePathChanged();
    void relativeFilePathChanged();

private:
    QString _absoluteFilePath;
    QString _relativeFilePath;
};
}
}
}
