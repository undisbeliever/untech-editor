/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "filesystemwatcher.h"
#include "abstractproject.h"
#include "abstractresourceitem.h"
#include "abstractresourcelist.h"
#include "models/common/imagecache.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>

using namespace UnTech::GuiQt;

FilesystemWatcher::FilesystemWatcher(AbstractProject* project)
    : QObject(project)
    , _project(project)
    , _watcher(new QFileSystemWatcher(this))
    , _projectWatcher(new QFileSystemWatcher(this))
    , _filesChangedDialogActive(false)
    , _filenameToItems()
    , _itemToFilenames()
{
    Q_ASSERT(project);

    watchProjectFile();

    connect(_watcher, &QFileSystemWatcher::fileChanged,
            this, &FilesystemWatcher::onFileChanged);

    connect(_projectWatcher, &QFileSystemWatcher::fileChanged,
            this, &FilesystemWatcher::onProjectFileChanged);

    connect(_project, &AbstractProject::filenameChanged,
            this, &FilesystemWatcher::watchProjectFile);

    connect(_project, &AbstractProject::aboutToSaveProject,
            this, &FilesystemWatcher::onAboutToSaveProject);

    connect(_project, &AbstractProject::resourceItemCreated,
            this, &FilesystemWatcher::onResourceItemCreated);

    connect(_project, &AbstractProject::selectedResourceChanged,
            this, &FilesystemWatcher::onSelectedResourceChanged);

    connect(_project, &AbstractProject::resourceItemAboutToBeRemoved,
            this, &FilesystemWatcher::removeResourceItem);
}

void FilesystemWatcher::watchProjectFile()
{
    QString fn = _project->filename();
    if (fn.isEmpty() == false) {
        fn = QDir::fromNativeSeparators(fn);
    }

    QStringList toRemove = _projectWatcher->files();
    toRemove << _projectWatcher->directories();

    if (toRemove.size() == 1 && toRemove.first() == fn) {
        // already watching project file
        return;
    }

    if (toRemove.isEmpty() == false) {
        _projectWatcher->removePaths(toRemove);
    }

    if (fn.isEmpty() == false && QFile::exists(fn)) {
        _projectWatcher->addPath(_project->filename());
    }
}

void FilesystemWatcher::onAboutToSaveProject()
{
    _filesSavedLocally.append(_project->filename());
}

void FilesystemWatcher::onProjectFileChanged(const QString& path)
{
    QString nativePath = QDir::toNativeSeparators(path);

    if (_filesSavedLocally.contains(nativePath)) {
        // Do not process items were saved by the editor
        _filesSavedLocally.removeAll(nativePath);

        // If the path was replaced we need to add it to the watcher again.
        watchProjectFile();
        return;
    }

    _project->undoStack()->resetClean();
    for (AbstractResourceList* rl : _project->resourceLists()) {
        for (AbstractResourceItem* item : rl->items()) {
            if (auto* inItem = qobject_cast<AbstractInternalResourceItem*>(item)) {
                inItem->undoStack()->resetClean();
            }
        }
    }

    QMessageBox::warning(
        nullptr, tr("Project File Changed"),
        tr("The project file \"%1\" has been changed on disk.\n"
           "Please save or discard changes manually.")
            .arg(QFileInfo(path).fileName()));

    watchProjectFile();
}

void FilesystemWatcher::onResourceItemCreated(AbstractResourceItem* item)
{
    updateWatcherAndMaps(item);

    connect(item, &AbstractResourceItem::externalFilesChanged,
            this, &FilesystemWatcher::onResourceItemExternalFilesChanged);

    if (auto* exItem = qobject_cast<AbstractExternalResourceItem*>(item)) {
        connect(exItem, &AbstractExternalResourceItem::aboutToSaveResource,
                this, &FilesystemWatcher::onAboutToSaveResource);
    }
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

static QStringList resourceNativeFilenames(const AbstractResourceItem* item)
{
    QStringList nativeFilenames = item->externalFiles();

    if (auto* exItem = qobject_cast<const AbstractExternalResourceItem*>(item)) {
        nativeFilenames << exItem->filename();
    }

    nativeFilenames.removeAll(QString());

    for (QString& fn : nativeFilenames) {
        fn = QDir::toNativeSeparators(QFileInfo(fn).absoluteFilePath());
    }

    return nativeFilenames;
}

void FilesystemWatcher::updateWatcherAndMaps(AbstractResourceItem* item)
{
    Q_ASSERT(item);

    const QStringList nativeFilenames = resourceNativeFilenames(item);
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

void FilesystemWatcher::onAboutToSaveResource()
{
    auto* item = qobject_cast<AbstractExternalResourceItem*>(sender());
    if (item) {
        _filesSavedLocally.append(item->filename());
    }
}

void FilesystemWatcher::onFileChanged(const QString& path)
{
    auto watchPathAgain = [&]() {
        // If the path was replaced (ie, file copy or atomic write) then the
        // watcher will automatically remove the path and we need to add it back
        // again.
        if (_watcher->files().contains(path) == false) {
            if (QFile::exists(path)) {
                _watcher->addPath(path);
            }
        }
    };

    QString nativePath = QDir::toNativeSeparators(path);

    ImageCache::invalidateFilename(nativePath.toStdString());

    if (_filesSavedLocally.contains(nativePath)) {
        // Do not process items were saved by the editor
        _filesSavedLocally.removeAll(nativePath);

        watchPathAgain();
        return;
    }

    auto it = _filenameToItems.find(nativePath);
    if (it != _filenameToItems.end()) {
        const auto& items = *it;

        for (AbstractResourceItem* item : items) {
            if (auto* exItem = qobject_cast<AbstractExternalResourceItem*>(item)) {
                if (nativePath == exItem->filename()) {
                    resourceChangedOnDisk(exItem);
                    continue;
                }
            }

            emit item->externalFilesModified();

            item->markUnchecked();
        }
    }
    else {
        qWarning() << nativePath << "is not linked to a ResourceItem";
    }

    watchPathAgain();
}

void FilesystemWatcher::resourceChangedOnDisk(AbstractExternalResourceItem* item)
{
    if (_changedResources.contains(item) == false) {
        _changedResources.append(item);
    }
    if (_filesChangedDialogActive == false) {
        showFilesChangedDialog();
    }
}

void FilesystemWatcher::showFilesChangedDialog()
{
    Q_ASSERT(_filesChangedDialogActive == false);
    _filesChangedDialogActive = true;

    QMessageBox dialog;

    dialog.setWindowTitle(tr("Changed Files"));
    dialog.addButton(QMessageBox::Yes);
    dialog.addButton(QMessageBox::No);
    dialog.addButton(QMessageBox::NoToAll);
    dialog.setDefaultButton(QMessageBox::No);

    {
        // Set size of dialog so the buttons are always in the same place.

        // QMessageDialog setSize methods do not work,
        // however setting the size of the label does.

        QSize labelSize(dialog.fontMetrics().width('m') * 50,
                        dialog.fontMetrics().height() * 5);

        for (QLabel* l : dialog.findChildren<QLabel*>()) {
            l->setFixedSize(labelSize);
        }
    }

    while (!_changedResources.empty()) {
        QPointer<AbstractExternalResourceItem> item = _changedResources.first();

        if (item) {
            dialog.setText(tr("The file \"%1\" has been changed on disk.\n"
                              "Do you want to reload it?\n\n"
                              "You will not be able to undo this action.")
                               .arg(item->relativeFilePath()));

            dialog.exec();

            switch (dialog.result()) {
            case QMessageBox::Yes:
                // required to prevent potential null pointer dereference warning in g++ release build
                if (AbstractExternalResourceItem* oldItem = item) {
                    auto* newItem = oldItem->resourceList()->revertResource(oldItem);
                    newItem->project()->setSelectedResource(newItem);
                }
                _changedResources.removeFirst();
                break;

            case QMessageBox::No:
                if (item) {
                    item->undoStack()->resetClean();
                }
                _changedResources.removeFirst();
                break;

            case QMessageBox::NoToAll:
                for (auto& i : _changedResources) {
                    if (i) {
                        i->undoStack()->resetClean();
                    }
                }
                _changedResources.clear();
                break;
            }
        }
    }

    _filesChangedDialogActive = false;
}
