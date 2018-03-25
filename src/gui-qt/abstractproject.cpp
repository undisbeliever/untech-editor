/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractproject.h"
#include "abstractresourceitem.h"
#include "abstractresourcelist.h"
#include "resourcevalidationworker.h"

#include <QFileInfo>
#include <QMessageBox>

using namespace UnTech::GuiQt;

AbstractProject::AbstractProject(QObject* parent)
    : QObject(parent)
    , _resourceLists()
    , _filename()
    , _undoStack(new QUndoStack(this))
    , _validationWorker(new ResourceValidationWorker(this))
    , _selectedResource(nullptr)
{
}

void AbstractProject::initResourceLists(std::initializer_list<AbstractResourceList*> resourceLists)
{
    _resourceLists = resourceLists;
}

void AbstractProject::setSelectedResource(AbstractResourceItem* item)
{
    if (_selectedResource != item) {
        if (_selectedResource) {
            _selectedResource->disconnect(this);
        }

        _selectedResource = item;

        if (_selectedResource) {
            connect(_selectedResource, &QObject::destroyed,
                    this, &AbstractProject::onSelectedResourceDestroyed);
        }

        emit selectedResourceChanged();
    }
}

bool AbstractProject::saveProject(const QString& filename)
{
    QString absFilename = QFileInfo(filename).absoluteFilePath();
    bool success = false;

    try {
        success = saveProjectFile(absFilename);
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error Saving File"), ex.what());
        success = false;
    }

    if (success) {
        if (_filename != absFilename) {
            _filename = absFilename;
            filenameChanged();
        }

        // Mark all internal resources as clean
        _undoStack->setClean();
        for (AbstractResourceList* rl : resourceLists()) {
            for (AbstractResourceItem* item : rl->items()) {
                if (auto* inItem = qobject_cast<AbstractInternalResourceItem*>(item)) {
                    inItem->undoStack()->setClean();
                }
            }
        }
    }

    return success;
}

bool AbstractProject::loadProject(const QString& filename)
{
    QString absFilename = QFileInfo(filename).absoluteFilePath();
    bool success = false;

    try {
        success = loadProjectFile(absFilename);
        if (success) {
            rebuildResourceLists();
        }
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error opening File"), ex.what());
        success = false;
    }

    if (success) {
        if (_filename != absFilename) {
            _filename = absFilename;
            filenameChanged();
        }
        _undoStack->clear();
    }

    return success;
}

void AbstractProject::onSelectedResourceDestroyed(QObject* obj)
{
    if (_selectedResource == obj) {
        setSelectedResource(nullptr);
    }
}

void AbstractProject::rebuildResourceLists()
{
    for (auto* ri : _resourceLists) {
        ri->rebuildResourceItems();
    }
}

QList<AbstractExternalResourceItem*> AbstractProject::unsavedExternalResources() const
{
    QList<AbstractExternalResourceItem*> items;

    for (AbstractResourceList* rl : _resourceLists) {
        for (AbstractResourceItem* item : rl->items()) {
            if (auto* exItem = qobject_cast<AbstractExternalResourceItem*>(item)) {
                if (exItem->undoStack()->isClean() == false) {
                    items.append(exItem);
                }
            }
        }
    }

    return items;
}

QStringList AbstractProject::unsavedFilenames() const
{
    QList<QString> filenames;
    bool resourceFileDirty = _undoStack->isClean() == false;

    for (AbstractResourceList* rl : _resourceLists) {
        for (AbstractResourceItem* item : rl->items()) {
            if (auto* exItem = qobject_cast<AbstractExternalResourceItem*>(item)) {
                if (exItem->undoStack()->isClean() == false) {
                    filenames.append(exItem->relativeFilePath());
                }
            }
            else {
                // internal resource
                if (item->undoStack()->isClean() == false) {
                    resourceFileDirty = true;
                }
            }
        }
    }

    if (resourceFileDirty) {
        filenames.prepend(QFileInfo(filename()).fileName());
    }

    return filenames;
}
