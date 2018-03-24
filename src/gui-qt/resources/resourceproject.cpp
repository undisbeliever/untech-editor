/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceproject.h"
#include "models/resources/resources-serializer.h"
#include "mttileset/mttilesetresourcelist.h"
#include "palette/paletteresourcelist.h"

using namespace UnTech::GuiQt::Resources;

ResourceProject::ResourceProject(QObject* parent)
    : AbstractProject(parent)
    , _resourcesFile(std::make_unique<RES::ResourcesFile>())
{
    initResourceLists({
        new PaletteResourceList(this, ResourceTypeIndex::PALETTE),
        new MtTilesetResourceList(this, ResourceTypeIndex::MT_TILESET),
    });
}

const QString& ResourceProject::fileFilter() const
{
    static const QString FILTER = QString::fromUtf8(
        "UnTech Resources File (*.utres);;All Files (*)");

    return FILTER;
}

const QString& ResourceProject::defaultFileExtension() const
{
    static const QString EXTENSION = QString::fromUtf8("utres");
    return EXTENSION;
}

bool ResourceProject::saveDocumentFile(const QString& filename)
{
    RES::saveResourcesFile(*_resourcesFile, filename.toUtf8().data());

    // Mark all internal resources as clean
    _undoStack->setClean();
    for (AbstractResourceList* rl : resourceLists()) {
        for (AbstractResourceItem* item : rl->items()) {
            if (auto* inItem = qobject_cast<AbstractInternalResourceItem*>(item)) {
                inItem->undoStack()->setClean();
            }
        }
    }

    return true;
}

bool ResourceProject::loadDocumentFile(const QString& filename)
{
    auto res = RES::loadResourcesFile(filename.toUtf8().data());
    if (res) {
        _resourcesFile = std::move(res);
        rebuildResourceLists();
        return true;
    }
    return false;
}
