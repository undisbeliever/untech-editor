/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QObject>
#include <QUndoStack>

namespace UnTech {
namespace GuiQt {
class AbstractResourceList;
class AbstractResourceItem;
class AbstractExternalResourceItem;
class ResourceValidationWorker;
class FilesystemWatcher;

class AbstractProject : public QObject {
    Q_OBJECT

public:
    explicit AbstractProject(QObject* parent = nullptr);
    ~AbstractProject() = default;

protected:
    void initResourceLists(std::initializer_list<AbstractResourceList*> resourceLists);

public:
    const QString& filename() const { return _filename; }
    QUndoStack* undoStack() const { return _undoStack; }
    const auto& resourceLists() const { return _resourceLists; }
    ResourceValidationWorker* validationWorker() const { return _validationWorker; }
    FilesystemWatcher* filesystemWatcher() const { return _filesystemWatcher; }

    void setSelectedResource(AbstractResourceItem* item);
    AbstractResourceItem* selectedResource() const { return _selectedResource; }

    bool saveProject(const QString& filename);
    bool loadProject(const QString& filename);

    QList<AbstractExternalResourceItem*> unsavedExternalResources() const;

    // All unsaved filenames are relative to the directory this project is saved to
    QStringList unsavedFilenames() const;

protected:
    // can throw exceptions
    virtual bool saveProjectFile(const QString& filename) = 0;
    virtual bool loadProjectFile(const QString& filename) = 0;

protected:
    void rebuildResourceLists();

private slots:
    void onSelectedResourceDestroyed(QObject* obj);

signals:
    void filenameChanged();
    void resourceFileSettingsChanged();

    void selectedResourceChanged();
    void resourceItemCreated(AbstractResourceItem*);
    void resourceItemAboutToBeRemoved(AbstractResourceItem*);

private:
    QList<AbstractResourceList*> _resourceLists;

    QString _filename;
    QUndoStack* const _undoStack;
    ResourceValidationWorker* const _validationWorker;
    FilesystemWatcher* const _filesystemWatcher;

    AbstractResourceItem* _selectedResource;
};
}
}
