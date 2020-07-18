/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "gui-qt/project.h"
#include "models/project/project-data.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles::InteractiveTiles;

ResourceItem::ResourceItem(StaticResourceList* list, unsigned index)
    : AbstractInternalResourceItem(list, index)
    , _functionTableList(new FunctionTableList(this))
{
    setName(tr("Interactive Tiles"));
    setRemovable(false);

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

bool ResourceItem::compileResource(UnTech::ErrorList& err)
{
    return project()->projectData().compileInteractiveTiles(err);
}