/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "resourcevalidationworker.h"
#include "models/resources/resources-serializer.h"
#include "mttileset/mttilesetresourcelist.h"
#include "palette/paletteresourcelist.h"

#include <QFileInfo>

using namespace UnTech::GuiQt::Resources;

// _resourceLists order MUST match ResourceTypeIndex

Document::Document(QObject* parent)
    : AbstractDocument(parent)
    , _resourcesFile(std::make_unique<RES::ResourcesFile>())
    , _resourceLists({ {
          new PaletteResourceList(this, ResourceTypeIndex::PALETTE),
          new MtTilesetResourceList(this, ResourceTypeIndex::MT_TILESET),
      } })
    , _validationWorker(new ResourceValidationWorker(this))
    , _selectedResource(nullptr)
{
    initModels();
}

void Document::initModels()
{
    for (auto& rl : _resourceLists) {
        rl->setDocument(this);

        connect(rl, &AbstractResourceList::resourceItemCreated,
                this, &Document::resourceItemCreated);
        connect(rl, &AbstractResourceList::resourceItemAboutToBeRemoved,
                this, &Document::resourceItemAboutToBeRemoved);
    }
}

void Document::setSelectedResource(AbstractResourceItem* item)
{
    if (_selectedResource != item) {
        if (_selectedResource) {
            _selectedResource->disconnect(this);
        }

        _selectedResource = item;

        if (_selectedResource) {
            connect(_selectedResource, &QObject::destroyed,
                    this, &Document::onSelectedResourceDestroyed);
        }

        emit selectedResourceChanged();
    }
}

void Document::onSelectedResourceDestroyed(QObject* obj)
{
    if (_selectedResource == obj) {
        setSelectedResource(nullptr);
    }
}

const QString& Document::fileFilter() const
{
    static const QString FILTER = QString::fromUtf8(
        "UnTech Resources File (*.utres);;All Files (*)");

    return FILTER;
}

const QString& Document::defaultFileExtension() const
{
    static const QString EXTENSION = QString::fromUtf8("utres");
    return EXTENSION;
}

bool Document::saveDocumentFile(const QString& filename)
{
    RES::saveResourcesFile(*_resourcesFile, filename.toUtf8().data());

    // Mark all internal resources as clean
    _undoStack->setClean();
    for (AbstractResourceList* rl : _resourceLists) {
        for (AbstractResourceItem* item : rl->items()) {
            if (auto* inItem = qobject_cast<AbstractInternalResourceItem*>(item)) {
                inItem->undoStack()->setClean();
            }
        }
    }

    return true;
}

bool Document::loadDocumentFile(const QString& filename)
{
    auto res = RES::loadResourcesFile(filename.toUtf8().data());
    if (res) {
        _resourcesFile = std::move(res);
        initModels();
        return true;
    }
    return false;
}

QList<AbstractExternalResourceItem*> Document::unsavedExternalResources() const
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

QStringList Document::unsavedFilenames() const
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
