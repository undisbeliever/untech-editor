/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "gui-qt/project.h"
#include "gui-qt/resources/scene-settings/resourceitem.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project-data.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources::Scenes;

ResourceItem::ResourceItem(StaticResourceList* list, unsigned index)
    : AbstractInternalResourceItem(list, index)
    , _sceneList(new SceneList(this))
{
    setName(tr("Scenes"));
    setRemovable(false);

    setDependencies({
        { ResourceTypeIndex::STATIC, list->sceneSettings()->name() },
        { ResourceTypeIndex::PALETTE, QString() },
        { ResourceTypeIndex::MT_TILESET, QString() },
        { ResourceTypeIndex::BACKGROUND_IMAGE, QString() },
    });

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

bool ResourceItem::compileResource(UnTech::ErrorList& err)
{
    return project()->projectData().compileScenes(err);
}
