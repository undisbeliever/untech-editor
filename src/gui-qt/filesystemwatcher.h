/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QFileSystemWatcher>
#include <QList>
#include <QMap>
#include <QObject>

namespace UnTech {
namespace GuiQt {
class AbstractProject;
class AbstractResourceItem;

class FilesystemWatcher : public QObject {
    Q_OBJECT

public:
    FilesystemWatcher(AbstractProject* project);
    ~FilesystemWatcher() = default;

private slots:
    void onResourceItemCreated(AbstractResourceItem* item);
    void onResourceItemAboutToBeRemoved(AbstractResourceItem* item);

    void onSelectedResourceChanged();

    void onResourceItemExternalFilesChanged();

    void onFileChanged(const QString& path);

private:
    void updateExternalFiles(AbstractResourceItem* item);
    void removeResourceItem(AbstractResourceItem* item);

    void updateWatcherAndMaps(AbstractResourceItem* item, const QStringList& filenames);

private:
    AbstractProject* const _project;
    QFileSystemWatcher* const _watcher;

    QHash<QString, QList<AbstractResourceItem*>> _filenameToItems;
    QHash<AbstractResourceItem*, QStringList> _itemToFilenames;
};
}
}
