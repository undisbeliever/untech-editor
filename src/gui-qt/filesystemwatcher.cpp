/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "filesystemwatcher.h"
#include "abstractproject.h"
#include "abstractresourceitem.h"
#include "models/common/imagecache.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

using namespace UnTech::GuiQt;

FilesystemWatcher::FilesystemWatcher(AbstractProject* project)
    : QObject(project)
    , _project(project)
    , _watcher(new QFileSystemWatcher(this))
    , _filenameToItems()
    , _itemToFilenames()
{
    Q_ASSERT(project);

    connect(_watcher, &QFileSystemWatcher::fileChanged,
            this, &FilesystemWatcher::onFileChanged);

    connect(_project, &AbstractProject::resourceItemCreated,
            this, &FilesystemWatcher::onResourceItemCreated);

    connect(_project, &AbstractProject::selectedResourceChanged,
            this, &FilesystemWatcher::onSelectedResourceChanged);

    connect(_project, &AbstractProject::resourceItemAboutToBeRemoved,
            this, &FilesystemWatcher::removeResourceItem);
}

void FilesystemWatcher::onResourceItemCreated(AbstractResourceItem* item)
{
    updateWatcherAndMaps(item);

    connect(item, &AbstractResourceItem::externalFilesChanged,
            this, &FilesystemWatcher::onResourceItemExternalFilesChanged);
}

void FilesystemWatcher::onSelectedResourceChanged()
{
    // Reload the watcher when the selected item is changed.
    // Just in case a file that did not exist is now existing.

    if (auto* item = _project->selectedResource()) {
        updateWatcherAndMaps(item);
    }
}

void FilesystemWatcher::onResourceItemExternalFilesChanged()
{
    auto* item = qobject_cast<AbstractResourceItem*>(sender());
    if (item) {
        updateWatcherAndMaps(item);
    }
}

static QStringList toNativeFilenames(const QStringList& filenames)
{
    QStringList nativeFilenames = filenames;
    nativeFilenames.removeAll(QString());

    for (QString& fn : nativeFilenames) {
        fn = QDir::toNativeSeparators(QFileInfo(fn).absoluteFilePath());
    }

    return nativeFilenames;
}

void FilesystemWatcher::updateWatcherAndMaps(AbstractResourceItem* item)
{
    Q_ASSERT(item);

    const QStringList nativeFilenames = toNativeFilenames(item->externalFiles());
    const QStringList watchedFiles = _watcher->files();
    const QStringList previousFilenames = _itemToFilenames.value(item);

    bool emitModified = previousFilenames != nativeFilenames;

    {
        // Check the watched/existing status of every filename.
        // It may not have existed in the past, but it could exist now.

        QStringList toWatch;
        toWatch.reserve(nativeFilenames.size());

        for (const QString& nativePath : nativeFilenames) {
            QString path = QDir::fromNativeSeparators(nativePath);

            bool fileUnwatched = watchedFiles.contains(path) == false;
            bool fileExists = QFile::exists(nativePath);

            if (fileUnwatched && fileExists) {
                toWatch << path;

                emitModified = true;
            }
            if (fileUnwatched || !fileExists) {
                ImageCache::invalidateFilename(nativePath.toStdString());
            }
        }

        if (toWatch.isEmpty() == false) {
            // This is more efficient then adding paths one at a time
            _watcher->addPaths(toWatch);
        }
    }

    QStringList toAdd = nativeFilenames;

    for (const QString& prevFilename : previousFilenames) {
        int i = toAdd.indexOf(prevFilename);
        if (i >= 0) {
            toAdd.removeAt(i);
        }
        else {
            removeFilenameItemMapping(prevFilename, item);
        }
    }

    for (const QString& filename : toAdd) {
        auto it = _filenameToItems.find(filename);
        if (it == _filenameToItems.end()) {
            it = _filenameToItems.insert(filename, {});
        }
        if (it->contains(item) == false) {
            it->append(item);
        }
    }

    if (nativeFilenames.isEmpty() == false) {
        _itemToFilenames.insert(item, nativeFilenames);
    }
    else {
        _itemToFilenames.remove(item);
    }

    if (emitModified) {
        emit item->externalFilesModified();

        item->markUnchecked();
    }
}

void FilesystemWatcher::removeResourceItem(AbstractResourceItem* item)
{
    Q_ASSERT(item);

    const QStringList filenames = _itemToFilenames.value(item);
    for (const QString& fn : filenames) {
        removeFilenameItemMapping(fn, item);
    }

    _itemToFilenames.remove(item);
}

void FilesystemWatcher::removeFilenameItemMapping(const QString& filename, AbstractResourceItem* item)
{
    auto it = _filenameToItems.find(filename);
    if (it != _filenameToItems.end()) {
        it->removeAll(item);
        if (it->isEmpty()) {
            _watcher->removePath(filename);

            _filenameToItems.remove(filename);
        }
    }
}

void FilesystemWatcher::onFileChanged(const QString& path)
{
    // If the path was replaced (ie, file copy or atomic write) then the
    // watcher will automatically remove the path and we need to add it back
    // again.
    if (_watcher->files().contains(path) == false) {
        if (QFile::exists(path)) {
            _watcher->addPath(path);
        }
    }

    QString nativePath = QDir::toNativeSeparators(path);

    ImageCache::invalidateFilename(nativePath.toStdString());

    auto it = _filenameToItems.find(nativePath);
    if (it != _filenameToItems.end()) {
        const auto& items = *it;

        for (AbstractResourceItem* item : items) {
            emit item->externalFilesModified();

            item->markUnchecked();
        }
    }
    else {
        qWarning() << nativePath << "is not linked to a ResourceItem";
    }
}
