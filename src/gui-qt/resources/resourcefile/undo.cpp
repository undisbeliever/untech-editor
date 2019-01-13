/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcefilepropertymanager.h"
#include "gui-qt/accessor/projectsettingsundohelper.h"
#include "gui-qt/resources/resourceproject.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Resources;

namespace RES = UnTech::Resources;

using SettingsUndoHelper = ProjectSettingsUndoHelper<ResourceProject>;

bool ResourceFilePropertyManager::editBlockSettings_setSize(unsigned blockSize)
{
    return SettingsUndoHelper(_project).editField(
        blockSize,
        tr("Edit Block Size"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.blockSettings.size; });
}

bool ResourceFilePropertyManager::editBlockSettings_setCount(unsigned blockCount)
{
    return SettingsUndoHelper(_project).editField(
        blockCount,
        tr("Edit Block Count"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.blockSettings.count; });
}

bool ResourceFilePropertyManager::editMetaTileSettings_setMaxMapSize(unsigned maxMapSize)
{
    return SettingsUndoHelper(_project).editField(
        maxMapSize,
        tr("Edit MetaTile Max Map Size"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.metaTileEngineSettings.maxMapSize; });
}

bool ResourceFilePropertyManager::editMetaTileSettings_setNMetaTiles(unsigned nMetaTiles)
{
    return SettingsUndoHelper(_project).editField(
        nMetaTiles,
        tr("Edit Number of MetaTiles"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.metaTileEngineSettings.nMetaTiles; });
}
