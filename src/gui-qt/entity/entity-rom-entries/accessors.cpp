/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/abstractaccessors.hpp"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Entity::EntityRomEntries;

template <>
const NamedList<EN::EntityRomEntry>* NamedListAccessor<EN::EntityRomEntry, ResourceItem>::list() const
{
    const auto* erel = qobject_cast<const EntityRomEntriesList*>(this);
    Q_ASSERT(erel);
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    if (erel->isEntityList()) {
        return &projectFile->entityRomData.entities;
    }
    else {
        return &projectFile->entityRomData.projectiles;
    }
}

template <>
NamedList<EN::EntityRomEntry>* NamedListAccessor<EN::EntityRomEntry, ResourceItem>::getList()
{
    auto* erel = qobject_cast<EntityRomEntriesList*>(this);
    Q_ASSERT(erel);
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    if (erel->isEntityList()) {
        return &projectFile->entityRomData.entities;
    }
    else {
        return &projectFile->entityRomData.projectiles;
    }
}

EntityRomEntriesList::EntityRomEntriesList(ResourceItem* resourceItem,
                                           bool entityList)
    : NamedListAccessor(resourceItem, 255)
    , _entityList(entityList)
{
}

QString EntityRomEntriesList::typeName() const
{
    return tr("Entity ROM Entry");
}

QString EntityRomEntriesList::typeNamePlural() const
{
    return tr("Entity ROM Entries");
}

void EntityRomEntriesList::editSelected_setFunctionTable(const idstring& functionTable)
{
    UndoHelper(this).editSelectedItemField(
        functionTable,
        tr("Edit Function Table"),
        [](DataT& s) -> idstring& { return s.functionTable; },
        [](EntityRomEntriesList* a, size_t i) { emit a->implementsChanged(i); });
}

void EntityRomEntriesList::editSelected_setComment(const std::string& comment)
{
    UndoHelper(this).editSelectedItemField(
        comment,
        tr("Edit Comment"),
        [](DataT& s) -> std::string& { return s.comment; });
}

bool EntityRomEntriesList::editSelected_setInitialProjectileId(const idstring& initialProjectileId)
{
    return UndoHelper(this).editSelectedItemField(
        initialProjectileId,
        tr("Edit initialProjectileId"),
        [](DataT& s) -> idstring& { return s.initialProjectileId; });
}

bool EntityRomEntriesList::editSelected_setInitialListId(const idstring& initialListId)
{
    return UndoHelper(this).editSelectedItemField(
        initialListId,
        tr("Edit initialListId"),
        [](DataT& s) -> idstring& { return s.initialListId; });
}

bool EntityRomEntriesList::editSelected_setFrameSetId(const idstring& frameSetId)
{
    return UndoHelper(this).editSelectedItemField(
        frameSetId,
        tr("Edit frameSetId"),
        [](DataT& s) -> idstring& { return s.frameSetId; });
}

bool EntityRomEntriesList::editSelected_setDisplayFrame(const idstring& displayFrame)
{
    return UndoHelper(this).editSelectedItemField(
        displayFrame,
        tr("Edit displayFrame"),
        [](DataT& s) -> idstring& { return s.displayFrame; });
}

bool EntityRomEntriesList::editSelected_setDefaultPalette(unsigned defaultPalette)
{
    return UndoHelper(this).editSelectedItemField(
        defaultPalette,
        tr("Edit defaultPalette"),
        [](DataT& s) -> unsigned& { return s.defaultPalette; });
}

bool EntityRomEntriesList::editSelected_setField(const QString& field, const std::string& value)
{
    const idstring fieldId(field.toStdString());

    return UndoHelper(this).editSelectedItemField(
        value,
        tr("Edit %1").arg(field),
        [=](DataT& s) -> std::string& { return s.fields[fieldId]; });
}

template class UnTech::GuiQt::Accessor::NamedListAccessor<EN::EntityRomEntry, ResourceItem>;
