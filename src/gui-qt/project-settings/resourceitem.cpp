/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/project.h"
#include "gui-qt/resourcevalidationworker.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt::ProjectSettings;

namespace PRO = UnTech::Project;

ResourceItem::ResourceItem(UnTech::GuiQt::StaticResourceList* list, unsigned index)
    : AbstractInternalResourceItem(list, index)
{
    setName(tr("Project Settings"));

    setRemovable(false);

    connect(this, &ResourceItem::dataChanged,
            project()->validationWorker(), &ResourceValidationWorker::validateAllResources);
}

bool ResourceItem::compileResource(UnTech::ErrorList& err)
{
    const auto* projectFile = project()->projectFile();
    Q_ASSERT(projectFile);

    bool valid = true;

    valid &= projectFile->blockSettings.validate(err);
    valid &= projectFile->entityRomData.validateListIds(err);

    return valid;
}

bool ResourceItem::editBlockSettings_setSize(unsigned blockSize)
{
    return UndoHelper(this).editField(
        blockSize,
        tr("Edit Block Size"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.blockSettings.size; });
}

bool ResourceItem::editBlockSettings_setCount(unsigned blockCount)
{
    return UndoHelper(this).editField(
        blockCount,
        tr("Edit Block Count"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.blockSettings.count; });
}

bool ResourceItem::editMetaTileSettings_setMaxMapSize(unsigned maxMapSize)
{
    return UndoHelper(this).editField(
        maxMapSize,
        tr("Edit MetaTile Max Map Size"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.metaTileEngineSettings.maxMapSize; });
}

bool ResourceItem::editMetaTileSettings_setNMetaTiles(unsigned nMetaTiles)
{
    return UndoHelper(this).editField(
        nMetaTiles,
        tr("Edit Number of MetaTiles"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.metaTileEngineSettings.nMetaTiles; });
}

bool ResourceItem::editEntityRomData_setEntityListIds(const std::vector<idstring>& listIds)
{
    return UndoHelper(this).editField(
        listIds,
        tr("Edit EntityListIds"),
        [](PRO::ProjectFile& pf) -> std::vector<idstring>& { return pf.entityRomData.listIds; },
        [](ResourceItem& item) { emit item.entityListIdsChanged(); });
}
