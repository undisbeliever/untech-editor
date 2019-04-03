/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/abstractaccessors.hpp"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Entity::EntityRomStructs;

template <>
const NamedList<EN::EntityRomStruct>* NamedListAccessor<EN::EntityRomStruct, ResourceItem>::list() const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->entityRomData.structs;
}

template <>
NamedList<EN::EntityRomStruct>* NamedListAccessor<EN::EntityRomStruct, ResourceItem>::getList()
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->entityRomData.structs;
}

EntityRomStructList::EntityRomStructList(ResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, 255)
{
}

QString EntityRomStructList::typeName() const
{
    return tr("Entity ROM Struct");
}

QString EntityRomStructList::typeNamePlural() const
{
    return tr("Entity ROM Structs");
}

void EntityRomStructList::editSelected_setParent(const idstring& parent)
{
    UndoHelper(this).editSelectedItemField(
        parent,
        tr("Edit Parent"),
        [](DataT& s) -> idstring& { return s.parent; },
        [](EntityRomStructList* a, index_type i) { emit a->parentChanged(i); });
}

void EntityRomStructList::editSelected_setComment(const std::string& comment)
{
    UndoHelper(this).editSelectedItemField(
        comment,
        tr("Edit Comment"),
        [](DataT& s) -> std::string& { return s.comment; },
        [](EntityRomStructList* a, index_type i) { emit a->commentChanged(i); });
}

template <>
const std::vector<EN::StructField>* ChildVectorMultipleSelectionAccessor<EN::StructField, ResourceItem>::list(size_t pIndex) const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    auto& structs = projectFile->entityRomData.structs;
    if (pIndex >= structs.size()) {
        return nullptr;
    }
    return &structs.at(pIndex).fields;
}

template <>
std::vector<EN::StructField>* ChildVectorMultipleSelectionAccessor<EN::StructField, ResourceItem>::getList(size_t pIndex)
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    auto& structs = projectFile->entityRomData.structs;
    if (pIndex >= structs.size()) {
        return nullptr;
    }
    return &structs.at(pIndex).fields;
}

EntityRomStructFieldList::EntityRomStructFieldList(EntityRomStructList* structList)
    : ChildVectorMultipleSelectionAccessor(structList, structList->resourceItem(), 24)
{
}

QString EntityRomStructFieldList::typeName() const
{
    return tr("Struct Field");
}

QString EntityRomStructFieldList::typeNamePlural() const
{
    return tr("Struct Fields");
}

bool EntityRomStructFieldList::editSelectedList_setName(size_t index, const idstring& name)
{
    return UndoHelper(this).editField(
        index, name,
        tr("Edit Field Name"),
        [](DataT& f) -> idstring& { return f.name; });
}

bool EntityRomStructFieldList::editSelectedList_setType(size_t index, EN::DataType type)
{
    return UndoHelper(this).editField(
        index, type,
        tr("Edit Field Type"),
        [](DataT& f) -> EN::DataType& { return f.type; });
}

bool EntityRomStructFieldList::editSelectedList_setDefaultValue(size_t index, const std::string& value)
{
    return UndoHelper(this).editField(
        index, value,
        tr("Edit Field Default Value"),
        [](DataT& f) -> std::string& { return f.defaultValue; });
}

bool EntityRomStructFieldList::editSelectedList_setComment(size_t index, const std::string& comment)
{
    return UndoHelper(this).editField(
        index, comment,
        tr("Edit Field Comment"),
        [](DataT& f) -> std::string& { return f.comment; });
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<EN::EntityRomStruct, ResourceItem>;
template class Accessor::ChildVectorMultipleSelectionAccessor<EN::StructField, ResourceItem>;
