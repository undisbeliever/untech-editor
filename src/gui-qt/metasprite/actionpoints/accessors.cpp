/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "actionpointsresourceitem.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "gui-qt/project.h"
#include "models/project/project.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::ActionPoints;

template <>
const NamedList<ActionPointFunction>* NamedListAccessor<ActionPointFunction, ActionPointsResourceItem>::list() const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->actionPointFunctions;
}

template <>
NamedList<ActionPointFunction>* NamedListAccessor<ActionPointFunction, ActionPointsResourceItem>::getList()
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->actionPointFunctions;
}

ActionPointFunctionsList::ActionPointFunctionsList(ActionPointsResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, 255)
{
}

QString ActionPointFunctionsList::typeName() const
{
    return tr("Action Point Function");
}

QString ActionPointFunctionsList::typeNamePlural() const
{
    return tr("Action Point Functions");
}

bool ActionPointFunctionsList::edit_setManuallyInvoked(size_t index, bool manuallyInvoked)
{
    return UndoHelper(this).editField(
        index, manuallyInvoked,
        manuallyInvoked ? tr("Set Manually Invoked") : tr("Clear Manually Invoked"),
        [](DataT& d) -> bool& { return d.manuallyInvoked; });
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<ActionPointFunction, ActionPointsResourceItem>;
