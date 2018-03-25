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

bool ResourceProject::saveProjectFile(const QString& filename)
{
    RES::saveResourcesFile(*_resourcesFile, filename.toUtf8().data());

    return true;
}

bool ResourceProject::loadProjectFile(const QString& filename)
{
    auto res = RES::loadResourcesFile(filename.toUtf8().data());
    if (res) {
        _resourcesFile = std::move(res);
        return true;
    }
    return false;
}
