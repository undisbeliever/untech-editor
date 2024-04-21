/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityromdata-serializer.h"
#include <cassert>

namespace UnTech::Entity {

static const EnumMap<EntityType> entityTypeMap = {
    { u8"entity", EntityType::ENTITY },
    { u8"projectile", EntityType::PROJECTILE },
    { u8"player", EntityType::PLAYER },
};

static const EnumMap<DataType> dataTypeMap = {
    { u8"uint8", DataType::UINT8 },
    { u8"uint16", DataType::UINT16 },
    { u8"uint24", DataType::UINT24 },
    { u8"uint32", DataType::UINT32 },
    { u8"sint8", DataType::SINT8 },
    { u8"sint16", DataType::SINT16 },
    { u8"sint24", DataType::SINT24 },
    { u8"sint32", DataType::SINT32 },
};

static const EnumMap<ParameterType> parameterTypeMap = {
    { u8"unused", ParameterType::UNUSED },
    { u8"unsigned", ParameterType::UNSIGNED_BYTE },
};

using namespace Xml;

static void readEntityRomStruct(XmlReader& xml, const XmlTag& tag, NamedList<EntityRomStruct>& structs)
{
    assert(tag.name() == u8"struct");

    structs.insert_back();
    EntityRomStruct& romStruct = structs.back();

    romStruct.name = tag.getAttributeOptionalId(u8"name");
    romStruct.parent = tag.getAttributeOptionalId(u8"parent");
    romStruct.comment = tag.getAttributeOrEmpty(u8"comment");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name() == u8"struct-field") {
            romStruct.fields.emplace_back(
                StructField{
                    childTag.getAttributeOptionalId(u8"name"),
                    childTag.getAttributeEnum(u8"type", dataTypeMap),
                    childTag.getAttributeOrEmpty(u8"default"),
                    childTag.getAttributeOrEmpty(u8"comment") });
        }
        else {
            throw unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }
}

static void readEntityFunctionTable(const XmlTag& tag, NamedList<EntityFunctionTable>& functionTables)
{
    assert(tag.name() == u8"function-table");

    functionTables.insert_back();
    EntityFunctionTable& ft = functionTables.back();

    ft.name = tag.getAttributeOptionalId(u8"name");
    if (tag.hasAttribute(u8"type")) {
        ft.entityType = tag.getAttributeEnum(u8"type", entityTypeMap);
    }
    ft.entityStruct = tag.getAttributeOptionalId(u8"struct");
    ft.exportOrder = tag.getAttributeOptionalId(u8"export-order");
    ft.parameterType = tag.getAttributeEnum(u8"parameter-type", parameterTypeMap);
    ft.comment = tag.getAttributeOrEmpty(u8"comment");
}

static void readEntityRomEntry(XmlReader& xml, const XmlTag& tag, const EntityType entityType,
                               NamedList<EntityRomEntry>& entries)
{
    assert(tag.name() == u8"entry");

    entries.insert_back();
    EntityRomEntry& entry = entries.back();

    entry.name = tag.getAttributeOptionalId(u8"name");
    entry.functionTable = tag.getAttributeOptionalId(u8"function-table");
    entry.comment = tag.getAttributeOrEmpty(u8"comment");
    entry.initialProjectileId = tag.getAttributeOptionalId(u8"projectileid");
    if (entityType != EntityType::PLAYER) {
        entry.initialListId = tag.getAttributeOptionalId(u8"listid");
    }
    entry.frameSetId = tag.getAttributeOptionalId(u8"frameset");
    entry.displayFrame = tag.getAttributeOptionalId(u8"frame");
    entry.defaultPalette = tag.getAttributeUnsigned(u8"palette");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name() == u8"entry-field") {
            idstring fieldFor = childTag.getAttributeId(u8"for");
            std::u8string fieldValue = childTag.getAttribute(u8"value");

            if (entry.fields.find(fieldFor) != entry.fields.end()) {
                throw xml_error(childTag, u8"Duplicate entry-field detected");
            }
            entry.fields.emplace(fieldFor, fieldValue);
        }
        else {
            throw unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }
}

static void readEntityRomEntries(XmlReader& xml, const EntityType entityType,
                                 NamedList<EntityRomEntry>& entries)
{
    while (const auto childTag = xml.parseTag()) {
        if (childTag.name() == u8"entry") {
            readEntityRomEntry(xml, childTag, entityType, entries);
        }
        else {
            throw unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }
}

void readEntityRomData(XmlReader& xml, const XmlTag& tag, EntityRomData& entityRomData)
{
    assert(tag.name() == u8"entity-rom-data");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name() == u8"listid") {
            entityRomData.listIds.push_back(childTag.getAttributeOptionalId(u8"name"));
        }
        else if (childTag.name() == u8"struct") {
            readEntityRomStruct(xml, childTag, entityRomData.structs);
        }
        else if (childTag.name() == u8"function-table") {
            readEntityFunctionTable(childTag, entityRomData.functionTables);
        }
        else if (childTag.name() == u8"entities") {
            readEntityRomEntries(xml, EntityType::ENTITY, entityRomData.entities);
        }
        else if (childTag.name() == u8"projectiles") {
            readEntityRomEntries(xml, EntityType::PROJECTILE, entityRomData.projectiles);
        }
        else if (childTag.name() == u8"players") {
            readEntityRomEntries(xml, EntityType::PLAYER, entityRomData.players);
        }
        else {
            throw unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }
}

void writeEntityRomEntries(XmlWriter& xml, const std::u8string& tagName, const EntityType entityType,
                           const NamedList<EntityRomEntry>& entries)
{
    if (entries.empty()) {
        return;
    }

    xml.writeTag(tagName);

    for (auto& entry : entries) {
        xml.writeTag(u8"entry");

        xml.writeTagAttributeOptional(u8"name", entry.name);
        xml.writeTagAttributeOptional(u8"function-table", entry.functionTable);
        xml.writeTagAttributeOptional(u8"comment", entry.comment);
        xml.writeTagAttributeOptional(u8"projectileid", entry.initialProjectileId);
        if (entityType != EntityType::PLAYER) {
            xml.writeTagAttributeOptional(u8"listid", entry.initialListId);
        }
        xml.writeTagAttributeOptional(u8"frameset", entry.frameSetId);
        xml.writeTagAttributeOptional(u8"frame", entry.displayFrame);
        xml.writeTagAttribute(u8"palette", entry.defaultPalette);

        // Sort fields so output is the same on all systems
        using FieldValue = std::unordered_map<idstring, std::u8string>::value_type;
        std::vector<std::reference_wrapper<const FieldValue>> fields(entry.fields.begin(), entry.fields.end());
        std::sort(fields.begin(), fields.end(),
                  [](const FieldValue& a, const FieldValue& b) { return a.first < b.first; });

        for (const auto& f : fields) {
            if (!f.get().second.empty()) {
                xml.writeTag(u8"entry-field");
                xml.writeTagAttribute(u8"for", f.get().first);
                xml.writeTagAttribute(u8"value", f.get().second);
                xml.writeCloseTag();
            }
        }

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

void writeEntityRomData(XmlWriter& xml, const EntityRomData& entityRomData)
{
    xml.writeTag(u8"entity-rom-data");

    for (auto& listId : entityRomData.listIds) {
        xml.writeTag(u8"listid");
        xml.writeTagAttribute(u8"name", listId);
        xml.writeCloseTag();
    }

    for (const auto& romStruct : entityRomData.structs) {
        xml.writeTag(u8"struct");
        xml.writeTagAttributeOptional(u8"name", romStruct.name);
        xml.writeTagAttributeOptional(u8"parent", romStruct.parent);
        xml.writeTagAttributeOptional(u8"comment", romStruct.comment);

        for (const auto& field : romStruct.fields) {
            xml.writeTag(u8"struct-field");
            xml.writeTagAttributeOptional(u8"name", field.name);
            xml.writeTagAttributeEnum(u8"type", field.type, dataTypeMap);
            xml.writeTagAttributeOptional(u8"default", field.defaultValue);
            xml.writeTagAttributeOptional(u8"comment", field.comment);
            xml.writeCloseTag();
        }
        xml.writeCloseTag();
    }

    for (const auto& ft : entityRomData.functionTables) {
        xml.writeTag(u8"function-table");
        xml.writeTagAttributeOptional(u8"name", ft.name);
        xml.writeTagAttributeEnum(u8"type", ft.entityType, entityTypeMap);
        xml.writeTagAttributeOptional(u8"struct", ft.entityStruct);
        xml.writeTagAttributeOptional(u8"export-order", ft.exportOrder);
        xml.writeTagAttributeEnum(u8"parameter-type", ft.parameterType, parameterTypeMap);
        xml.writeTagAttributeOptional(u8"comment", ft.comment);
        xml.writeCloseTag();
    }

    writeEntityRomEntries(xml, u8"entities", EntityType::ENTITY, entityRomData.entities);
    writeEntityRomEntries(xml, u8"projectiles", EntityType::PROJECTILE, entityRomData.projectiles);
    writeEntityRomEntries(xml, u8"players", EntityType::PLAYER, entityRomData.players);

    xml.writeCloseTag();
}

}
