/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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
class ExportOrderResourceList;
}
}
namespace MetaTiles {
namespace MtTileset {
class MtTilesetResourceList;
}
}
namespace Resources {
namespace Palette {
class PaletteResourceList;
}
}

class Project : public QObject {
    Q_OBJECT

public:
    using DataT = UnTech::Project::ProjectFile;

private:
    Project(std::unique_ptr<DataT> projectFile, QString filename);

public:
    virtual ~Project() final;

public:
    static std::unique_ptr<Project> newProject(const QString& filenmae);
    static std::unique_ptr<Project> loadProject(const QString& filename);

    const QString& filename() const { return _filename; }
    QUndoStack* undoStack() const { return _undoStack; }
    const auto& resourceLists() const { return _resourceLists; }
    ResourceValidationWorker* validationWorker() const { return _validationWorker; }
    FilesystemWatcher* filesystemWatcher() const { return _filesystemWatcher; }

    UnTech::Project::ProjectFile* projectFile() const { return _projectFile.get(); }

    StaticResourceList* staticResourceList() const { return _staticResourceList; }
    MetaSprite::ExportOrder::ExportOrderResourceList* frameSetExportOrderResourceList() const { return _frameSetExportOrderResourceList; }
    MetaSprite::FrameSetResourceList* frameSetResourceList() const { return _frameSetResourceList; }
    Resources::Palette::PaletteResourceList* paletteResourceList() const { return _paletteResourceList; }
    MetaTiles::MtTileset::MtTilesetResourceList* mtTilesetResourceList() const { return _mtTilesetResourceList; }

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

    QString _filename;
    QUndoStack* const _undoStack;
    ResourceValidationWorker* const _validationWorker;
    FilesystemWatcher* const _filesystemWatcher;

    StaticResourceList* const _staticResourceList;
    MetaSprite::ExportOrder::ExportOrderResourceList* const _frameSetExportOrderResourceList;
    MetaSprite::FrameSetResourceList* const _frameSetResourceList;
    Resources::Palette::PaletteResourceList* const _paletteResourceList;
    MetaTiles::MtTileset::MtTilesetResourceList* const _mtTilesetResourceList;

    const QList<AbstractResourceList*> _resourceLists;

    AbstractResourceItem* _selectedResource;
};
}
}
