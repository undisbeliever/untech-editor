/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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

    valid &= projectFile->memoryMap.validate(err);
    valid &= projectFile->roomSettings.validate(err);
    valid &= projectFile->entityRomData.validateListIds(err);

    return valid;
}

bool ResourceItem::editMemoryMap_setMappingMode(PRO::MappingMode mode)
{
    return UndoHelper(this).editField(
        mode,
        tr("Edit Mapping Mode"),
        [](PRO::ProjectFile& pf) -> PRO::MappingMode& { return pf.memoryMap.mode; });
}

bool ResourceItem::editBlockSettings_setFirstBank(unsigned firstBank)
{
    return UndoHelper(this).editField(
        firstBank,
        tr("Edit First Bank"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.memoryMap.firstBank; });
}

bool ResourceItem::editBlockSettings_setNBanks(unsigned int nBanks)
{
    return UndoHelper(this).editField(
        nBanks,
        tr("Edit Number of Banks"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.memoryMap.nBanks; });
}

bool ResourceItem::editMetaTileSettings_setRoomDataSize(unsigned roomDataSize)
{
    return UndoHelper(this).editField(
        roomDataSize,
        tr("Edit Room Data Size"),
        [](PRO::ProjectFile& pf) -> unsigned& { return pf.roomSettings.roomDataSize; });
}

bool ResourceItem::editEntityRomData_setEntityListIds(const std::vector<idstring>& listIds)
{
    return UndoHelper(this).editField(
        listIds,
        tr("Edit EntityListIds"),
        [](PRO::ProjectFile& pf) -> std::vector<idstring>& { return pf.entityRomData.listIds; },
        [](ResourceItem& item) { emit item.entityListIdsChanged(); });
}
