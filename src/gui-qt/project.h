/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include <QObject>
#include <QUndoStack>
#include <memory>

namespace UnTech {
namespace Project {
struct ProjectFile;
class ProjectData;
}

namespace GuiQt {
class AbstractResourceList;
class AbstractResourceItem;
class AbstractExternalResourceItem;
class ResourceValidationWorker;
class FilesystemWatcher;

class StaticResourceList;
namespace MetaSprite {
class FrameSetResourceList;
namespace ExportOrder {
class ResourceList;
}
}
namespace MetaTiles {
namespace MtTileset {
class ResourceList;
}
}
namespace Resources {
namespace Palette {
class ResourceList;
}
namespace BackgroundImage {
class ResourceList;
}
}
namespace Rooms {
class ResourceList;
}

class Project : public QObject {
    Q_OBJECT

public:
    using DataT = UnTech::Project::ProjectFile;

private:
    Project(std::unique_ptr<DataT> projectFile, QString filename);

public:
    virtual ~Project();

public:
    static std::unique_ptr<Project> newProject(const QString& filenmae);
    static std::unique_ptr<Project> loadProject(const QString& filename);

    const QString& filename() const { return _filename; }
    QUndoStack* undoStack() const { return _undoStack; }
    const auto& resourceLists() const { return _resourceLists; }
    ResourceValidationWorker* validationWorker() const { return _validationWorker; }
    FilesystemWatcher* filesystemWatcher() const { return _filesystemWatcher; }

    UnTech::Project::ProjectFile* projectFile() const { return _projectFile.get(); }
    UnTech::Project::ProjectData& projectData() const { return *_projectData; }

    StaticResourceList* staticResources() const { return _staticResources; }
    MetaSprite::ExportOrder::ResourceList* frameSetExportOrders() const { return _frameSetExportOrders; }
    MetaSprite::FrameSetResourceList* frameSets() const { return _frameSets; }
    Resources::Palette::ResourceList* palettes() const { return _palettes; }
    Resources::BackgroundImage::ResourceList* backgroundImages() const { return _backgroundImages; }
    MetaTiles::MtTileset::ResourceList* mtTilesets() const { return _mtTilesets; }
    Rooms::ResourceList* rooms() const { return _rooms; }

    void setSelectedResource(AbstractResourceItem* item);
    AbstractResourceItem* selectedResource() const { return _selectedResource; }

    AbstractResourceList* findResourceList(ResourceTypeIndex type) const;
    AbstractResourceItem* findResourceItem(ResourceTypeIndex type, const QString& name) const;

    bool saveProject(const QString& filename);

    QList<AbstractExternalResourceItem*> unsavedExternalResources() const;

    // All unsaved filenames are relative to the directory this project is saved to
    QStringList unsavedFilenames() const;

private slots:
    void onSelectedResourceDestroyed(QObject* obj);

signals:
    void aboutToSaveProject();

    void filenameChanged();

    void selectedResourceChanged();
    void resourceItemCreated(AbstractResourceItem*);
    void resourceItemAboutToBeRemoved(AbstractResourceItem*);

private:
    std::unique_ptr<UnTech::Project::ProjectFile> const _projectFile;
    std::unique_ptr<UnTech::Project::ProjectData> const _projectData;

    QString _filename;
    QUndoStack* const _undoStack;
    ResourceValidationWorker* const _validationWorker;
    FilesystemWatcher* const _filesystemWatcher;

    StaticResourceList* const _staticResources;
    MetaSprite::ExportOrder::ResourceList* const _frameSetExportOrders;
    MetaSprite::FrameSetResourceList* const _frameSets;
    Resources::Palette::ResourceList* const _palettes;
    Resources::BackgroundImage::ResourceList* const _backgroundImages;
    MetaTiles::MtTileset::ResourceList* const _mtTilesets;
    Rooms::ResourceList* const _rooms;

    const QList<AbstractResourceList*> _resourceLists;

    AbstractResourceItem* _selectedResource;
};
}
}
