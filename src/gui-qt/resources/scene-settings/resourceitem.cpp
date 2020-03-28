/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "gui-qt/project.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources::SceneSettings;

ResourceItem::ResourceItem(StaticResourceList* list, unsigned index)
    : AbstractInternalResourceItem(list, index)
    , _sceneSettingsList(new SceneSettingsList(this))
{
    setName(tr("Scene Settings"));
    setRemovable(false);

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

bool ResourceItem::compileResource(UnTech::ErrorList& err)
{
    using namespace UnTech::Resources;

    const auto* projectFile = project()->projectFile();
    Q_ASSERT(projectFile);
    const auto& sceneSettings = projectFile->resourceScenes.settings;

    auto oldErrorCount = err.errorCount();

    _sceneSettingsData = RES::compileSceneSettingsData(sceneSettings, err);

    return _sceneSettingsData && err.errorCount() == oldErrorCount;
}
