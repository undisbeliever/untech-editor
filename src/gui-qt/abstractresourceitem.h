/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractresourcelist.h"
#include "models/resources/error-list.h"
#include <QObject>
#include <QUndoStack>

namespace UnTech {
namespace GuiQt {
class AbstractProject;
class AbstractResourceList;

namespace RES = UnTech::Resources;

class AbstractResourceItem : public QObject {
    Q_OBJECT
    friend class AbstractInternalResourceItem;

public:
    AbstractResourceItem(AbstractResourceList* parent, unsigned index)
        : QObject(parent)
        , _list(parent)
        , _project(parent->project())
        , _undoStack(new QUndoStack(this))
        , _index(index)
        , _state(ResourceState::NOT_LOADED)
    {
        Q_ASSERT(parent != nullptr);
        Q_ASSERT(_project != nullptr);

        connect(this, &AbstractResourceItem::dataChanged,
                this, &AbstractResourceItem::markUnchecked);
    }

    ~AbstractResourceItem() = default;

    AbstractProject* project() const { return _project; }
    QUndoStack* undoStack() const { return _undoStack; }

    inline int index() const { return _index; }
    const QString& name() const { return _name; }
    const ResourceState& state() const { return _state; }
    const RES::ErrorList& errorList() const { return _errorList; }

    ResourceTypeIndex resourceTypeIndex() const { return _list->resourceTypeIndex(); }
    AbstractResourceList* resourceList() const { return _list; }

    // empty if the resource is inline with the resource project file
    virtual QString filename() const = 0;

    void validateItem();

public slots:
    void markUnchecked();
    void loadResource();

protected:
    // MUST be called by the subclass when the resource name changes
    void setName(const QString& name);

    virtual bool loadResourceData(RES::ErrorList& err) = 0;

    // compiles the resource to test if valid
    virtual bool compileResource(RES::ErrorList& err) = 0;

signals:
    // MUST be emitted by the subclass when the internal data changes
    void dataChanged();

    // Will be always be emitted at the end of loadResource,
    // (even if loadResourceData() failed).
    void resourceLoaded();

private:
    friend void AbstractResourceList::removeResource(int);
    void setIndex(unsigned index);

private:
    void setState(ResourceState state);

signals:
    void nameChanged();
    void stateChanged();
    void errorListChanged();

protected:
    AbstractResourceList* const _list;
    AbstractProject* const _project;

private:
    QUndoStack* const _undoStack;

    unsigned _index;
    QString _name;
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

    // will raise exception on error
    void saveResource() const;

protected:
    void setFilename(const QString& filename);

    // may raise an exception on error
    virtual void saveResourceData(const std::string& filename) const = 0;

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
