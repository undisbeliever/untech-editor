/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/listundohelper.h"
#include "gui-qt/accessor/namedlistundohelper.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Entity;

template class UnTech::GuiQt::Accessor::NamedListAndSelectionUndoHelper<EntityRomStructList>;

using StructListUndoHelper = NamedListAndSelectionUndoHelper<EntityRomStructList>;

void EntityRomStructList::editSelected_setName(const idstring& name)
{
    StructListUndoHelper(this).renameSelectedItem(name);
}

void EntityRomStructList::editSelected_setParent(const idstring& parent)
{
    StructListUndoHelper(this).editSelectedItemField(
        parent,
        tr("Edit Parent"),
        [](DataT& s) -> idstring& { return s.parent; },
        [](EntityRomStructList* a, index_type i) { emit a->parentChanged(i); });
}

void EntityRomStructList::editSelected_setComment(const std::string& comment)
{
    StructListUndoHelper(this).editSelectedItemField(
        comment,
        tr("Edit Comment"),
        [](DataT& s) -> std::string& { return s.comment; },
        [](EntityRomStructList* a, index_type i) { emit a->commentChanged(i); });
}

using StructFieldUndoHelper = ListUndoHelper<EntityRomStructFieldList>;

bool EntityRomStructFieldList::editSelectedList_setName(size_t index, const idstring& name)
{
    return StructFieldUndoHelper(this).editFieldInSelectedList(
        index, name,
        tr("Edit Field Name"),
        [](DataT& f) -> idstring& { return f.name; });
}

bool EntityRomStructFieldList::editSelectedList_setType(size_t index, EN::DataType type)
{
    return StructFieldUndoHelper(this).editFieldInSelectedList(
        index, type,
        tr("Edit Field Type"),
        [](DataT& f) -> EN::DataType& { return f.type; });
}

bool EntityRomStructFieldList::editSelectedList_setDefaultValue(size_t index, const std::string& value)
{
    return StructFieldUndoHelper(this).editFieldInSelectedList(
        index, value,
        tr("Edit Field Default Value"),
        [](DataT& f) -> std::string& { return f.defaultValue; });
}

bool EntityRomStructFieldList::editSelectedList_setComment(size_t index, const std::string& comment)
{
    return StructFieldUndoHelper(this).editFieldInSelectedList(
        index, comment,
        tr("Edit Field Comment"),
        [](DataT& f) -> std::string& { return f.comment; });
}
