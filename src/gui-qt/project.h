/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "gui-qt/accessor/accessor.h"
#include "models/project/project.h"
#include <QObject>
#include <QUndoStack>
#include <memory>

namespace UnTech {
namespace GuiQt {
class AbstractResourceList;
class AbstractResourceItem;
class AbstractExternalResourceItem;
class ResourceValidationWorker;
class FilesystemWatcher;

namespace MetaSprite {
class ExportOrderResourceList;
class FrameSetResourceList;
}
namespace MetaTiles {
class MtTilesetResourceList;
}
namespace Resources {
class PaletteResourceList;
}

class Project : public QObject {
    Q_OBJECT

public:
    using DataT = UnTech::Project::ProjectFile;

public:
    explicit Project(QObject* parent = nullptr);
    virtual ~Project() final;

protected:
    void initResourceLists(std::initializer_list<AbstractResourceList*> resourceLists);

public:
    const QString& filename() const { return _filename; }
    QUndoStack* undoStack() const { return _undoStack; }
    const auto& resourceLists() const { return _resourceLists; }
    ResourceValidationWorker* validationWorker() const { return _validationWorker; }
    FilesystemWatcher* filesystemWatcher() const { return _filesystemWatcher; }

    UnTech::Project::ProjectFile* projectFile() const { return _projectFile.get(); }

    MetaSprite::ExportOrderResourceList* frameSetExportOrderResourceList() const { return _frameSetExportOrderResourceList; }
    MetaSprite::FrameSetResourceList* frameSetResourceList() const { return _frameSetResourceList; }
    Resources::PaletteResourceList* paletteResourceList() const { return _paletteResourceList; }
    MetaTiles::MtTilesetResourceList* mtTilesetResourceList() const { return _mtTilesetResourceList; }

    void setSelectedResource(AbstractResourceItem* item);
    AbstractResourceItem* selectedResource() const { return _selectedResource; }

    AbstractResourceList* findResourceList(ResourceTypeIndex type);
    AbstractResourceItem* findResourceItem(ResourceTypeIndex type, const QString& name) const;

    bool saveProject(const QString& filename);
    bool loadProject(const QString& filename);

    QList<AbstractExternalResourceItem*> unsavedExternalResources() const;

    // All unsaved filenames are relative to the directory this project is saved to
    QStringList unsavedFilenames() const;

protected:
    friend class Accessor::ProjectSettingsUndoHelper<Project>;
    DataT* dataEditable() const { return _projectFile.get(); }

protected:
    void rebuildResourceLists();

private slots:
    void onSelectedResourceDestroyed(QObject* obj);

signals:
    void aboutToSaveProject();

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

    std::unique_ptr<UnTech::Project::ProjectFile> _projectFile;

    MetaSprite::ExportOrderResourceList* const _frameSetExportOrderResourceList;
    MetaSprite::FrameSetResourceList* const _frameSetResourceList;
    Resources::PaletteResourceList* const _paletteResourceList;
    MetaTiles::MtTilesetResourceList* const _mtTilesetResourceList;

    AbstractResourceItem* _selectedResource;
};
}
}
