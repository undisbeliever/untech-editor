/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QFileSystemWatcher>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QObject>
#include <QPointer>

namespace UnTech {
namespace GuiQt {
class Project;
class AbstractResourceItem;
class AbstractExternalResourceItem;

class FilesystemWatcher : public QObject {
    Q_OBJECT

public:
    FilesystemWatcher(Project* project);
    ~FilesystemWatcher() = default;

private slots:
    void watchProjectFile();
    void onAboutToSaveProject();
    void onProjectFileChanged(const QString& path);

    void onResourceItemCreated(AbstractResourceItem* item);

    void onSelectedResourceChanged();
    void onResourceItemExternalFilesChanged();

    void removeResourceItem(AbstractResourceItem* item);

    void onAboutToSaveResource();

    void onFileChanged(const QString& path);

private:
    void updateWatcherAndMaps(AbstractResourceItem* item);
    void removeFilenameItemMapping(const QString& filename, AbstractResourceItem* item);

    void resourceChangedOnDisk(AbstractExternalResourceItem*);
    void showFilesChangedDialog();

private:
    Project* const _project;
    QFileSystemWatcher* const _watcher;
    QFileSystemWatcher* const _projectWatcher;
    bool _filesChangedDialogActive;

    QHash<QString, QList<AbstractResourceItem*>> _filenameToItems;
    QHash<AbstractResourceItem*, QStringList> _itemToFilenames;

    QList<QPointer<AbstractExternalResourceItem>> _changedResources;
    QStringList _filesSavedLocally;
};
}
}
