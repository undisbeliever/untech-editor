/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceproject.h"
#include "gui-qt/metatiles/mttileset/mttilesetresourcelist.h"
#include "models/resources/resources-serializer.h"
#include "palette/paletteresourcelist.h"

using namespace UnTech::GuiQt::Resources;

ResourceProject::ResourceProject(QObject* parent)
    : AbstractProject(parent)
    , _resourcesFile(std::make_unique<PRO::ProjectFile>())
{
    initResourceLists({
        new PaletteResourceList(this),
        new MetaTiles::MtTilesetResourceList(this),
    });

    Q_ASSERT(paletteResourceList());
}

PaletteResourceList* ResourceProject::paletteResourceList() const
{
    return qobject_cast<PaletteResourceList*>(resourceLists().at(PALETTE_LIST_INDEX));
}

bool ResourceProject::saveProjectFile(const QString& filename)
{
    PRO::saveProjectFile(*_resourcesFile, filename.toUtf8().data());

    return true;
}

bool ResourceProject::loadProjectFile(const QString& filename)
{
    auto res = PRO::loadProjectFile(filename.toUtf8().data());
    if (res) {
        _resourcesFile = std::move(res);
        return true;
    }
    return false;
}
