/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/abstractaccessors.hpp"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Entity::EntityFunctionTables;

template <>
const NamedList<EN::EntityFunctionTable>* NamedListAccessor<EN::EntityFunctionTable, ResourceItem>::list() const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->entityRomData.functionTables;
}

template <>
NamedList<EN::EntityFunctionTable>* NamedListAccessor<EN::EntityFunctionTable, ResourceItem>::getList()
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->entityRomData.functionTables;
}

EntityFunctionTableList::EntityFunctionTableList(ResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, 255)
{
}

QString EntityFunctionTableList::typeName() const
{
    return tr("Entity Function Table");
}

QString EntityFunctionTableList::typeNamePlural() const
{
    return tr("Entity Function Tables");
}

bool EntityFunctionTableList::edit_setEntityType(index_type index, const EN::EntityType entityType)
{
    return UndoHelper(this).editField(
        index, entityType,
        tr("Edit Entity Type"),
        [](DataT& s) -> EN::EntityType& { return s.entityType; });
}

bool EntityFunctionTableList::edit_setExportOrder(index_type index, const idstring& exportOrder)
{
    return UndoHelper(this).editField(
        index, exportOrder,
        tr("Edit FrameSet Export Order"),
        [](DataT& s) -> idstring& { return s.exportOrder; });
}

bool EntityFunctionTableList::edit_setParameterType(index_type index, EN::ParameterType parameterType)
{
    return UndoHelper(this).editField(
        index, parameterType,
        tr("Edit Entity Parameter Type"),
        [](DataT& s) -> EN::ParameterType& { return s.parameterType; });
}

bool EntityFunctionTableList::edit_setEntityStruct(index_type index, const idstring& entityStruct)
{
    return UndoHelper(this).editField(
        index, entityStruct,
        tr("Edit Entity Struct"),
        [](DataT& s) -> idstring& { return s.entityStruct; });
}

bool EntityFunctionTableList::edit_setComment(index_type index, const std::string& comment)
{
    return UndoHelper(this).editField(
        index, comment,
        tr("Edit Comment"),
        [](DataT& s) -> std::string& { return s.comment; });
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<EN::EntityFunctionTable, ResourceItem>;
