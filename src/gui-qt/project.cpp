/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project.h"
#include "abstractresourceitem.h"
#include "abstractresourcelist.h"
#include "filesystemwatcher.h"
#include "project-data-slots.hpp"
#include "resourcevalidationworker.h"
#include "models/project/project-data.h"
#include "models/project/project.h"

#include "staticresourcelist.h"
#include "gui-qt/metasprite/exportorder/resourcelist.h"
#include "gui-qt/metasprite/framesetresourcelist.h"
#include "gui-qt/metatiles/mttileset/resourcelist.h"
#include "gui-qt/resources/background-image/resourcelist.h"
#include "gui-qt/resources/palette/resourcelist.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

using namespace UnTech::GuiQt;

Project::Project(std::unique_ptr<Project::DataT> projectFile, QString filename)
    : QObject()
    , _projectFile(std::move(projectFile))
    , _projectData(std::make_unique<UnTech::Project::ProjectData>(*_projectFile))
    , _filename(filename)
    , _undoStack(new QUndoStack(this))
    , _validationWorker(new ResourceValidationWorker(this))
    , _filesystemWatcher(new FilesystemWatcher(this))
    , _staticResources(new StaticResourceList(this))
    , _frameSetExportOrders(new MetaSprite::ExportOrder::ResourceList(this))
    , _frameSets(new MetaSprite::FrameSetResourceList(this))
    , _palettes(new Resources::Palette::ResourceList(this))
    , _backgroundImages(new Resources::BackgroundImage::ResourceList(this))
    , _mtTilesets(new MetaTiles::MtTileset::ResourceList(this))
    , _resourceLists({
          _staticResources,
          _frameSetExportOrders,
          _frameSets,
          _palettes,
          _backgroundImages,
          _mtTilesets,
      })
    , _selectedResource(nullptr)
{
    Q_ASSERT(_projectFile);
    Q_ASSERT(_filename.isEmpty() == false);

    for (AbstractResourceList* rl : _resourceLists) {
        connect(rl, &AbstractResourceList::resourceItemCreated,
                this, &Project::resourceItemCreated);
        connect(rl, &AbstractResourceList::resourceItemAboutToBeRemoved,
                this, &Project::resourceItemAboutToBeRemoved);

        rl->rebuildResourceItems();
    }

    ProjectDataSlots::connect(this);
}

Project::~Project()
{
    // Force cleanup of ResourceItems.
    // This is done is the Project instead of in ResourceList to ensure that
    // the `AbstractProject::resourceItemAboutToBeRemoved` signal is used.

    setSelectedResource(nullptr);

    for (auto* rl : _resourceLists) {
        for (auto* item : rl->items()) {
            emit rl->resourceItemAboutToBeRemoved(item);
        }
    }
}

void Project::setSelectedResource(AbstractResourceItem* item)
{
    if (_selectedResource != item) {
        if (_selectedResource) {
            _selectedResource->disconnect(this);
        }

        // Unselect the current resource before selecting the new one.
        // This is to ensure the GUI is cleared when changing items and because
        // it prevents a qFatal in `Accessor::MultiListActions`
        if (item != nullptr) {
            _selectedResource = nullptr;
            emit selectedResourceChanged();
        }

        _selectedResource = item;

        if (_selectedResource) {
            connect(_selectedResource, &QObject::destroyed,
                    this, &Project::onSelectedResourceDestroyed);
        }

        emit selectedResourceChanged();
    }
}

AbstractResourceList* Project::findResourceList(ResourceTypeIndex type) const
{
    switch (type) {
    case ResourceTypeIndex::STATIC:
        return _staticResources;

    case ResourceTypeIndex::PALETTE:
        return _palettes;

    case ResourceTypeIndex::MT_TILESET:
        return _mtTilesets;

    case ResourceTypeIndex::MS_FRAMESET:
        return _frameSets;

    case ResourceTypeIndex::MS_EXPORT_ORDER:
        return _frameSetExportOrders;

    case ResourceTypeIndex::BACKGROUND_IMAGE:
        return _backgroundImages;
    }

    qFatal("Unknown ResourceTypeIndex type");
}

AbstractResourceItem* Project::findResourceItem(ResourceTypeIndex type, const QString& name) const
{
    return findResourceList(type)->findResource(name);
}

bool Project::saveProject(const QString& filename)
{
    using namespace UnTech::Project;

    Q_ASSERT(_projectFile);

    emit aboutToSaveProject();

    QString absFilename = QDir::toNativeSeparators(
        QFileInfo(filename).absoluteFilePath());

    bool success = false;
    try {
        saveProjectFile(*_projectFile, filename.toUtf8().data());
        success = true;
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

std::unique_ptr<Project> Project::newProject(const QString& filenmae)
{
    std::unique_ptr<Project> project(new Project(std::make_unique<DataT>(), filenmae));

    bool s = project->saveProject(filenmae);
    if (s) {
        return project;
    }
    else {
        return nullptr;
    }
}

std::unique_ptr<Project> Project::loadProject(const QString& filename)
{
    using namespace UnTech::Project;

    QString absFilename = QDir::toNativeSeparators(
        QFileInfo(filename).absoluteFilePath());

    try {
        auto pf = loadProjectFile(absFilename.toUtf8().data());
        if (pf) {
            return std::unique_ptr<Project>(new Project(std::move(pf), absFilename));
        }
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error opening File"), ex.what());
        return nullptr;
    }

    QMessageBox::critical(nullptr, tr("Error opening File"), tr("Unknown error"));
    return nullptr;
}

void Project::onSelectedResourceDestroyed(QObject* obj)
{
    if (_selectedResource == obj) {
        setSelectedResource(nullptr);
    }
}

QList<AbstractExternalResourceItem*> Project::unsavedExternalResources() const
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

QStringList Project::unsavedFilenames() const
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
