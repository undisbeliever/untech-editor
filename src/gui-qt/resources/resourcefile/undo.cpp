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

bool ResourceProject::editBlockSettings_setSize(unsigned blockSize)
{
    return SettingsUndoHelper(this).editField(
        blockSize,
        tr("Edit Block Size"),
        [](RES::ResourcesFile& rf) -> unsigned& { return rf.blockSettings.size; });
}

bool ResourceProject::editBlockSettings_setCount(unsigned blockCount)
{
    return SettingsUndoHelper(this).editField(
        blockCount,
        tr("Edit Block Count"),
        [](RES::ResourcesFile& rf) -> unsigned& { return rf.blockSettings.count; });
}

bool ResourceProject::editMetaTileSettings_setMaxMapSize(unsigned maxMapSize)
{
    return SettingsUndoHelper(this).editField(
        maxMapSize,
        tr("Edit MetaTile Max Map Size"),
        [](RES::ResourcesFile& rf) -> unsigned& { return rf.metaTileEngineSettings.maxMapSize; });
}

bool ResourceProject::editMetaTileSettings_setNMetaTiles(unsigned nMetaTiles)
{
    return SettingsUndoHelper(this).editField(
        nMetaTiles,
        tr("Edit Number of MetaTiles"),
        [](RES::ResourcesFile& rf) -> unsigned& { return rf.metaTileEngineSettings.nMetaTiles; });
}
