/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractresourcelist.h"
#include "models/common/errorlist.h"
#include <QObject>
#include <QUndoStack>

namespace UnTech {
namespace GuiQt {
class AbstractProject;
class AbstractResourceList;

class AbstractResourceItem : public QObject {
    Q_OBJECT
    friend class AbstractInternalResourceItem;

public:
    struct Dependency {
        ResourceTypeIndex type;
        QString name;

        bool operator==(const Dependency& d) const { return type == d.type && name == d.name; }
        bool operator!=(const Dependency& d) const { return !(*this == d); }
    };

public:
    AbstractResourceItem(AbstractResourceList* parent, unsigned index)
        : QObject(parent)
        , _list(parent)
        , _project(parent->project())
        , _undoStack(new QUndoStack(this))
        , _index(index)
        , _state(ResourceState::NOT_LOADED)
        , _dependencies()
        , _externalFiles()
    {
        Q_ASSERT(parent != nullptr);
        Q_ASSERT(_project != nullptr);
    }

    ~AbstractResourceItem() = default;

    AbstractProject* project() const { return _project; }
    QUndoStack* undoStack() const { return _undoStack; }

    inline int index() const { return _index; }
    const QString& name() const { return _name; }
    const ResourceState& state() const { return _state; }
    const ErrorList& errorList() const { return _errorList; }
    const QVector<Dependency>& dependencies() const { return _dependencies; }
    const QStringList& externalFiles() const { return _externalFiles; }

    ResourceTypeIndex resourceTypeIndex() const { return _list->resourceTypeIndex(); }
    AbstractResourceList* resourceList() const { return _list; }

    // empty if the resource is inline with the resource project file
    virtual QString filename() const = 0;

    void validateItem();
    void markDependantsUnchecked();

public slots:
    // Marks the current item as unchecked.
    // Subclasses MUST invoke the `markChecked` slot when the resource item
    // becomes stale and needs to be recompiled.
    // This is usually done by connecting the subclass's dataChanged signal to
    // the markUnchecked slot.
    void markUnchecked();

private:
    friend class MainWindow;
    friend class ResourceValidationWorker;
    void loadResource();

protected:
    // MUST be called by the subclass when the resource name changes
    void setName(const QString& name);

    // MUST be called by the subclass when the dependencies change
    void setDependencies(const QVector<Dependency>& dependencies);
    void removeDependencies();

    // MUST be called when
    void setExternalFiles(const QStringList&);

    virtual bool loadResourceData(ErrorList& err) = 0;

    // Compiles the resource to test if it is valid.
    //
    // If the compiled resource is thrown away after compilation then this
    // function is allowed the preform bare minimum required to confirm the
    // resource will compile successfully.
    virtual bool compileResource(ErrorList& err) = 0;

signals:
    void externalFilesChanged();

    // Emitted by FilesystemWatcher when the externalFiles list changes or when
    // the external files have been modified by the filesystem.
    void externalFilesModified();

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
    void resourceComplied();

protected:
    AbstractResourceList* const _list;
    AbstractProject* const _project;

private:
    QUndoStack* const _undoStack;

    unsigned _index;
    QString _name;
    ResourceState _state;
    ErrorList _errorList;
    QVector<Dependency> _dependencies;
    QStringList _externalFiles;
};

class AbstractInternalResourceItem : public AbstractResourceItem {
    Q_OBJECT

public:
    AbstractInternalResourceItem(AbstractResourceList* parent, unsigned index);

    virtual QString filename() const final;

protected:
    virtual bool loadResourceData(ErrorList&) final;
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
    void saveResource();

protected:
    void setFilename(const QString& filename);

    // may raise an exception on error
    virtual void saveResourceData(const std::string& filename) const = 0;

private slots:
    void updateRelativePath();

signals:
    void absoluteFilePathChanged();
    void relativeFilePathChanged();

    void aboutToSaveResource();

private:
    QString _absoluteFilePath;
    QString _relativeFilePath;
};
}
}
