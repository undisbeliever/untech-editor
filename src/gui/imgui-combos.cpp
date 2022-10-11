/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui-combos.h"
#include "imgui.h"
#include "models/common/iterators.h"

namespace ImGui {

const std::array<const char*, 4> flipsComboItems = {
    "",
    "hFlip",
    "vFlip",
    "hvFlip"
};

template <typename EnumT, size_t N>
bool EnumCombo(const char* label, EnumT* value, const std::array<const char*, N>& items, int height_in_items = -1)
{
    static_assert(std::is_same_v<std::underlying_type_t<EnumT>, int>);

    int v = static_cast<int>(*value);

    bool c = ImGui::Combo(label, &v, items.data(), items.size(), height_in_items);
    if (c) {
        if (*value != static_cast<EnumT>(v)) {
            *value = static_cast<EnumT>(v);
            return true;
        }
    }
    return false;
}

bool EnumCombo(const char* label, UnTech::MetaSprite::Animation::DurationFormat* v)
{
    static constexpr auto items = std::to_array({
        "FRAME",
        "TIME",
        "DISTANCE_VERTICAL",
        "DISTANCE_HORIZONTAL",
    });
    return ImGui::EnumCombo(label, v, items);
}

bool EnumCombo(const char* label, UnTech::MetaSprite::TilesetType* v)
{
    static constexpr auto items = std::to_array({
        "ONE_TILE_FIXED",
        "TWO_TILES_FIXED",
        "ONE_ROW_FIXED",
        "TWO_ROWS_FIXED",
        "ONE_TILE",
        "TWO_TILES",
        "ONE_ROW",
        "TWO_ROWS",
    });
    return ImGui::EnumCombo(label, v, items);
}

bool EnumCombo(const char* label, UnTech::MetaSprite::ObjectSize* v)
{
    static constexpr auto items = std::to_array({
        "Small",
        "Large",
    });
    return ImGui::EnumCombo(label, v, items);
}

bool EnumCombo(const char* label, UnTech::MetaSprite::SpriteImporter::UserSuppliedPalette::Position* v)
{
    static constexpr auto items = std::to_array({
        "Top Left",
        "Top Right",
        "Bottom Left",
        "Bottom Right",
    });
    return ImGui::EnumCombo(label, v, items);
}

bool EnumCombo(const char* label, UnTech::Rooms::RoomEntranceOrientation* v)
{
    static constexpr auto items = std::to_array({
        "Down Right",
        "Down Left",
        "Up Right",
        "Up Left",
    });
    return ImGui::EnumCombo(label, v, items);
}

static constexpr auto dataTypeItems = std::to_array({
    "uint8",
    "uint16",
    "uint24",
    "uint32",
    "sint8",
    "sint16",
    "sint24",
    "sint32",
});

bool EnumCombo(const char* label, UnTech::Entity::DataType* v)
{
    return ImGui::EnumCombo(label, v, dataTypeItems);
}

void TextEnum(const UnTech::Entity::DataType& type)
{
    const unsigned i = static_cast<unsigned>(type);
    const char* str = i < dataTypeItems.size() ? dataTypeItems.at(i) : "";
    TextUnformatted(str);
}

static const auto argumentTypeItems = std::to_array({
    "",
    "Flag",
    "Word",
    "Immediate u16",
    "Room Script",
    "Entity Group",
    "Room",
    "Room Entrance",
});

bool EnumCombo(const char* label, UnTech::Scripting::ArgumentType* v)
{
    return ImGui::EnumCombo(label, v, argumentTypeItems);
}

void TextEnum(const UnTech::Scripting::ArgumentType& v)
{
    const unsigned i = static_cast<unsigned>(v);
    const char* str = i < argumentTypeItems.size() ? argumentTypeItems.at(i) : "";
    TextUnformatted(str);
}

bool EnumCombo(const char* label, UnTech::Scripting::ConditionalType* v)
{
    static const auto items = std::to_array({
        "word",
        "flag",
    });
    return ImGui::EnumCombo(label, v, items);
}

bool EnumCombo(const char* label, UnTech::Scripting::ComparisonType* v, UnTech::Scripting::ConditionalType t)
{
    using CT = UnTech::Scripting::ConditionalType;

    static const auto items = std::to_array({ "==",
                                              "!=",
                                              "<",
                                              ">=",
                                              "set",
                                              "clear" });

    bool edited = false;

    const unsigned index = unsigned(*v);

    auto sel = [&](const unsigned start, const unsigned end) {
        assert(start < end);
        assert(end <= items.size());

        for (const auto i : range(start, end)) {
            if (ImGui::Selectable(items.at(i), i == index)) {
                *v = UnTech::Scripting::ComparisonType(i);
                edited = true;
            }
        }
    };

    const char* text = index < items.size() ? items.at(index) : "";

    if (ImGui::BeginCombo(label, text)) {
        switch (t) {
        case CT::Word: {
            sel(0, 4);
        } break;

        case CT::Flag: {
            sel(4, 6);
        } break;
        }
        ImGui::EndCombo();
    }
    return edited;
}

bool EnumCombo(const char* label, UnTech::Snes::BitDepth* v)
{
    using BitDepth = UnTech::Snes::BitDepth;

    const char* text = "---";
    switch (*v) {
    case BitDepth::BD_2BPP:
        text = "2bpp";
        break;

    case BitDepth::BD_4BPP:
        text = "4bpp";
        break;

    case BitDepth::BD_8BPP:
        text = "8bpp";
        break;
    }

    bool edited = false;
    auto sel = [&](UnTech::Snes::BitDepth bd, const char* str) {
        if (ImGui::Selectable(str, bd == *v)) {
            *v = bd;
            edited = true;
        }
    };

    if (ImGui::BeginCombo(label, text)) {
        sel(BitDepth::BD_2BPP, "2bpp");
        sel(BitDepth::BD_4BPP, "4bpp");
        sel(BitDepth::BD_8BPP, "8bpp");

        ImGui::EndCombo();
    }

    return edited;
}

bool EnumCombo(const char* label, UnTech::Entity::EntityType* v)
{
    static const auto items = std::to_array({
        "Entity",
        "Projectile",
        "Player",
    });
    return ImGui::EnumCombo(label, v, items);
}

bool EnumCombo(const char* label, UnTech::Entity::ParameterType* v)
{
    static const auto items = std::to_array({
        "unused",
        "unsigned byte",
    });
    return ImGui::EnumCombo(label, v, items);
}

bool EnumCombo(const char* label, UnTech::Project::MappingMode* v)
{
    static const auto items = std::to_array({
        "LoROM",
        "HiROM",
    });
    return ImGui::EnumCombo(label, v, items);
}

bool EnumCombo(const char* label, UnTech::Resources::BgMode* v)
{
    static const auto items = std::to_array({
        "Mode 0",
        "Mode 1",
        "Mode 1 (bg3 priotity)",
        "Mode 2",
        "Mode 3",
        "Mode 4",
    });
    return ImGui::EnumCombo(label, v, items);
}

bool EnumCombo(const char* label, UnTech::Resources::LayerType* v)
{
    static const auto items = std::to_array({
        "None",
        "Background Image",
        "MetaTile Tileset",
        "Text Console",
    });
    return ImGui::EnumCombo(label, v, items);
}

bool BeginCombo(const char* label, const std::u8string& current, ImGuiComboFlags flags)
{
    return BeginCombo(label, u8Cast(current), flags);
}

bool IdStringCombo(const char* label, UnTech::idstring* value, const std::vector<UnTech::idstring>& list, bool includeBlank)
{
    return IdStringCombo(label, value, list, includeBlank,
                         [](const UnTech::idstring& name) { return &name; });
}

}
