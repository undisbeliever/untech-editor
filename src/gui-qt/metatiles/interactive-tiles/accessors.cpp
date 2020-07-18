/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "resourceitem.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "models/project/project.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaTiles::InteractiveTiles;

template <>
const NamedList<MT::InteractiveTileFunctionTable>* NamedListAccessor<MT::InteractiveTileFunctionTable, ResourceItem>::list() const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->interactiveTiles.functionTables;
}

template <>
NamedList<MT::InteractiveTileFunctionTable>* NamedListAccessor<MT::InteractiveTileFunctionTable, ResourceItem>::getList()
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->interactiveTiles.functionTables;
}

FunctionTableList::FunctionTableList(ResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, MT::MAX_INTERACTIVE_TILE_FUNCTION_TABLES - MT::InteractiveTiles::FIXED_FUNCTION_TABLES.size())
{
}

QString FunctionTableList::typeName() const
{
    return tr("Interactive Tile Function Table");
}

QString FunctionTableList::typeNamePlural() const
{
    return tr("Interactive Tile Function Table");
}

bool FunctionTableList::edit_setSymbol(size_t index, const std::string& symbol)
{
    return UndoHelper(this).editField(
        index, symbol,
        tr("Change Symbol"),
        [](MT::InteractiveTileFunctionTable& itf) -> std::string& { return itf.symbol; });
}

bool FunctionTableList::edit_setSymbolColor(size_t index, const rgba& symbolColor)
{
    return UndoHelper(this).editField(
        index, symbolColor,
        tr("Change Symbol Color"),
        [](MT::InteractiveTileFunctionTable& itf) -> rgba& { return itf.symbolColor; });
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<MT::InteractiveTileFunctionTable, ResourceItem>;
