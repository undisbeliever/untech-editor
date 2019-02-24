/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "projectsettingspropertymanager.h"
#include "projectsettingsresourceitem.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/project.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::ProjectSettings;

namespace PRO = UnTech::Project;

using SettingsUndoHelper = ResourceItemUndoHelper<ProjectSettingsResourceItem>;

bool ProjectSettingsPropertyManager::editBlockSettings_setSize(unsigned blockSize)
{
    return SettingsUndoHelper(_item).editField(
        blockSize,
        tr("Edit Block Size"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.blockSettings.size; });
}

bool ProjectSettingsPropertyManager::editBlockSettings_setCount(unsigned blockCount)
{
    return SettingsUndoHelper(_item).editField(
        blockCount,
        tr("Edit Block Count"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.blockSettings.count; });
}

bool ProjectSettingsPropertyManager::editMetaTileSettings_setMaxMapSize(unsigned maxMapSize)
{
    return SettingsUndoHelper(_item).editField(
        maxMapSize,
        tr("Edit MetaTile Max Map Size"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.metaTileEngineSettings.maxMapSize; });
}

bool ProjectSettingsPropertyManager::editMetaTileSettings_setNMetaTiles(unsigned nMetaTiles)
{
    return SettingsUndoHelper(_item).editField(
        nMetaTiles,
        tr("Edit Number of MetaTiles"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.metaTileEngineSettings.nMetaTiles; });
}

bool ProjectSettingsPropertyManager::editEntityRomData_setEntityListIds(const std::vector<idstring>& listIds)
{
    return SettingsUndoHelper(_item).editField(
        listIds,
        tr("Edit EntityListIds"),
        [](PRO::ProjectFile& pf) -> std::vector<idstring>& { return pf.entityRomData.listIds; },
        [](ProjectSettingsResourceItem& item) { emit item.entityListIdsChanged(); });
}
