/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "projectsettingsresourceitem.h"
#include "gui-qt/project.h"
#include "gui-qt/resourcevalidationworker.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt::ProjectSettings;

ProjectSettingsResourceItem::ProjectSettingsResourceItem(UnTech::GuiQt::StaticResourceList* list, unsigned index)
    : AbstractInternalResourceItem(list, index)
{
    setName(tr("Project Settings"));

    setRemovable(false);

    connect(this, &ProjectSettingsResourceItem::dataChanged,
            project()->validationWorker(), &ResourceValidationWorker::validateAllResources);
}

bool ProjectSettingsResourceItem::compileResource(UnTech::ErrorList& err)
{
    const auto* projectFile = project()->projectFile();
    Q_ASSERT(projectFile);

    bool valid = true;

    valid &= projectFile->blockSettings.validate(err);

    return valid;
}
