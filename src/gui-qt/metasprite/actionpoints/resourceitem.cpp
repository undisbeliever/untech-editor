/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::ActionPoints;

ResourceItem::ResourceItem(StaticResourceList* list, unsigned index)
    : AbstractInternalResourceItem(list, index)
    , _actionPointFunctionsList(new ActionPointFunctionsList(this))
{
    setName(tr("MetaSprite Action Points"));
    setRemovable(false);

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

QStringList ResourceItem::actionPointNames() const
{
    return actionPointFunctionsList()->itemNames();
}

bool ResourceItem::compileResource(UnTech::ErrorList& err)
{
    using namespace UnTech::MetaSprite;

    const auto* projectFile = project()->projectFile();
    Q_ASSERT(projectFile);

    const auto oldErrorCount = err.errorCount();

    _actionPointMapping = generateActionPointMapping(projectFile->actionPointFunctions, err);

    return oldErrorCount == err.errorCount();
}
