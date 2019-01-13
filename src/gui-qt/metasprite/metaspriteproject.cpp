/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metaspriteproject.h"
#include "framesetresourcelist.h"
#include "gui-qt/metasprite/exportorder/exportorderresourcelist.h"

using namespace UnTech::GuiQt::MetaSprite;

MetaSpriteProject::MetaSpriteProject(QObject* parent)
    : AbstractProject(parent)
    , _metaSpriteProject(std::make_unique<UnTech::Project::ProjectFile>())
{
    initResourceLists({
        new ExportOrderResourceList(this),
        new FrameSetResourceList(this),
    });
}

bool MetaSpriteProject::saveProjectFile(const QString& filename)
{
    UnTech::Project::saveProjectFile(*_metaSpriteProject, filename.toUtf8().data());

    return true;
}

bool MetaSpriteProject::loadProjectFile(const QString& filename)
{
    auto pro = UnTech::Project::loadProjectFile(filename.toUtf8().data());
    if (pro) {
        _metaSpriteProject = std::move(pro);
        return true;
    }
    return false;
}
