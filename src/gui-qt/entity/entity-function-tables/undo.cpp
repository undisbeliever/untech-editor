/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "entityfunctiontablesmanager.h"
#include "gui-qt/accessor/listundohelper.h"
#include "gui-qt/accessor/namedlistundohelper.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Entity;

using FTUndoHelper = NamedListUndoHelper<EntityFunctionTableList>;

bool EntityFunctionTableList::edit_setName(index_type index, const idstring& name)
{
    return FTUndoHelper(this).renameItem(index, name);
}

bool EntityFunctionTableList::edit_setExportOrder(EntityFunctionTableList::index_type index, const idstring& exportOrder)
{
    return FTUndoHelper(this).editField(
        index, exportOrder,
        tr("Edit FrameSet Export Order"),
        [](DataT& s) -> idstring& { return s.exportOrder; });
}

bool EntityFunctionTableList::edit_setParameterType(EntityFunctionTableList::index_type index, EN::ParameterType parameterType)
{
    return FTUndoHelper(this).editField(
        index, parameterType,
        tr("Edit Entity Parameter Type"),
        [](DataT& s) -> EN::ParameterType& { return s.parameterType; });
}

bool EntityFunctionTableList::edit_setEntityStruct(index_type index, const idstring& entityStruct)
{
    return FTUndoHelper(this).editField(
        index, entityStruct,
        tr("Edit Entity Struct"),
        [](DataT& s) -> idstring& { return s.entityStruct; });
}

bool EntityFunctionTableList::edit_setComment(index_type index, const std::string& comment)
{
    return FTUndoHelper(this).editField(
        index, comment,
        tr("Edit Comment"),
        [](DataT& s) -> std::string& { return s.comment; });
}

bool EntityFunctionTablesManager::insertItem(int index)
{
    if (_ftList == nullptr) {
        return false;
    }
    return FTUndoHelper(_ftList).addItem(index);
}

bool EntityFunctionTablesManager::cloneItem(int index)
{
    if (_ftList == nullptr) {
        return false;
    }
    return FTUndoHelper(_ftList).cloneItem(index);
}

bool EntityFunctionTablesManager::removeItem(int index)
{
    if (_ftList == nullptr) {
        return false;
    }
    return FTUndoHelper(_ftList).removeItem(index);
}

bool EntityFunctionTablesManager::moveItem(int from, int to)
{
    if (_ftList == nullptr) {
        return false;
    }
    return FTUndoHelper(_ftList).moveItem(from, to);
}
