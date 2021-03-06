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

// ::TODO only allow increment by one enums in enumMap::
// ::TODO remove enumMap enums::
template <typename EnumT>
bool oldStyleEnumClassCombo(const char* label, EnumT* v)
{
    bool changed = false;
    if (ImGui::BeginCombo(label, v->string())) {
        for (const auto& e : v->enumMap) {
            ImGui::PushID(static_cast<int>(e.second));

            if (ImGui::Selectable(e.first.c_str(), *v == e.second)) {
                *v = e.second;
                changed = true;
            }

            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    return changed;
}

template <typename EnumT>
bool EnumCombo(const char* label, EnumT* value, const char* const items[], int items_count, int height_in_items = -1)
{
    int v = static_cast<int>(*value);

    bool c = ImGui::Combo(label, &v, items, items_count, height_in_items);
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
    // ::TODO convert DurationFormat to a regular enum::
    return oldStyleEnumClassCombo(label, v);
}

bool EnumCombo(const char* label, UnTech::MetaSprite::TilesetType* v)
{
    // ::TODO convert TilesetType to a regular enum::
    return oldStyleEnumClassCombo(label, v);
}

bool EnumCombo(const char* label, UnTech::MetaSprite::ObjectSize* v)
{
    using ObjectSize = UnTech::MetaSprite::ObjectSize;

    static const char* smallText = "Small";
    static const char* largeText = "Large";

    const char* text = *v == ObjectSize::SMALL ? smallText : largeText;

    bool changed = false;
    if (ImGui::BeginCombo(label, text)) {
        auto sel = [&](const char* t, ObjectSize s) {
            if (ImGui::Selectable(t, *v == s)) {
                if (*v != s) {
                    *v = s;
                    changed = true;
                }
            }
        };
        sel(smallText, ObjectSize::SMALL);
        sel(largeText, ObjectSize::LARGE);

        ImGui::EndCombo();
    }

    return changed;
}

bool EntityHitboxTypeCombo(const char* label, UnTech::MetaSprite::EntityHitboxType* value)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;

    const unsigned currentIndex = value->romValue();

    bool changed = false;
    if (ImGui::BeginCombo(label, value->to_string())) {
        for (auto [i, ssv] : enumerate(EHT::SHORT_STRING_VALUES)) {
            if (ImGui::Selectable(ssv.c_str(), i == currentIndex)) {
                if (i != currentIndex) {
                    *value = EHT::from_romValue(i);
                    changed = true;
                }
            }
        }

        ImGui::EndCombo();
    }
    return changed;
}

bool EnumCombo(const char* label, UnTech::MetaSprite::SpriteImporter::UserSuppliedPalette::Position* v)
{
    static constexpr const char* items[] = {
        "Top Left",
        "Top Right",
        "Bottom Left",
        "Bottom Right",
    };
    return ImGui::EnumCombo(label, v, items, IM_ARRAYSIZE(items));
}

bool EnumCombo(const char* label, UnTech::Rooms::RoomEntranceOrientation* v)
{
    static constexpr const char* items[] = {
        "Down Right",
        "Down Left",
        "Up Right",
        "Up Left",
    };
    return ImGui::EnumCombo(label, v, items, IM_ARRAYSIZE(items));
}

static const char* const dataTypeItems[] = {
    "uint8",
    "uint16",
    "uint24",
    "uint32",
    "sint8",
    "sint16",
    "sint24",
    "sint32",
};

bool EnumCombo(const char* label, UnTech::Entity::DataType* v)
{
    return ImGui::EnumCombo(label, v, dataTypeItems, IM_ARRAYSIZE(dataTypeItems));
}

void TextEnum(const UnTech::Entity::DataType& type)
{
    const unsigned i = static_cast<unsigned>(type);
    const char* str = i < IM_ARRAYSIZE(dataTypeItems) ? dataTypeItems[i] : "";
    TextUnformatted(str);
}

static const char* const argumentTypeItems[] = {
    "",
    "Flag",
    "Word",
    "Immediate u16",
    "Room Script",
    "Entity Group",
    "Room",
    "Room Entrance",
};

bool EnumCombo(const char* label, UnTech::Scripting::ArgumentType* v)
{
    return ImGui::EnumCombo(label, v, argumentTypeItems, IM_ARRAYSIZE(argumentTypeItems));
}

void TextEnum(const UnTech::Scripting::ArgumentType& v)
{
    const unsigned i = static_cast<unsigned>(v);
    const char* str = i < IM_ARRAYSIZE(argumentTypeItems) ? argumentTypeItems[i] : "";
    TextUnformatted(str);
}

bool EnumCombo(const char* label, UnTech::Scripting::ConditionalType* v)
{
    static const char* const items[] = {
        "word",
        "flag",
    };
    return ImGui::EnumCombo(label, v, items, IM_ARRAYSIZE(items));
}

bool EnumCombo(const char* label, UnTech::Scripting::ComparisonType* v, UnTech::Scripting::ConditionalType t)
{
    using CT = UnTech::Scripting::ConditionalType;

    static const char* const items[] = {
        "==",
        "!=",
        "<",
        ">=",
        "set",
        "clear"
    };

    bool edited = false;

    const unsigned index = unsigned(*v);

    auto sel = [&](const unsigned start, const unsigned end) {
        assert(start < end);
        assert(end <= IM_ARRAYSIZE(items));

        for (const auto i : range(start, end)) {
            if (ImGui::Selectable(items[i], i == index)) {
                *v = UnTech::Scripting::ComparisonType(i);
                edited = true;
            }
        }
    };

    const char* text = index < IM_ARRAYSIZE(items) ? items[index] : "";

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

bool EnumCombo(const char* label, UnTech::Entity::EntityType* v)
{
    static const char* const items[] = {
        "Entity",
        "Projectile",
        "Player",
    };
    return ImGui::EnumCombo(label, v, items, IM_ARRAYSIZE(items));
}

bool EnumCombo(const char* label, UnTech::Entity::ParameterType* v)
{
    static const char* const items[] = {
        "unused",
        "unsigned byte",
    };
    return ImGui::EnumCombo(label, v, items, IM_ARRAYSIZE(items));
}

bool EnumCombo(const char* label, UnTech::Project::MappingMode* v)
{
    static const char* const items[] = {
        "LoROM",
        "HiROM",
    };
    return ImGui::EnumCombo(label, v, items, IM_ARRAYSIZE(items));
}

bool EnumCombo(const char* label, UnTech::Resources::BgMode* v)
{
    static const char* const items[] = {
        "Mode 0",
        "Mode 1",
        "Mode 1 (bg3 priotity)",
        "Mode 2",
        "Mode 3",
        "Mode 4",
    };
    return ImGui::EnumCombo(label, v, items, IM_ARRAYSIZE(items));
}

bool EnumCombo(const char* label, UnTech::Resources::LayerType* v)
{
    static const char* const items[] = {
        "None",
        "Background Image",
        "MetaTile Tileset",
        "Text Console",
    };
    return ImGui::EnumCombo(label, v, items, IM_ARRAYSIZE(items));
}

bool BeginCombo(const char* label, const std::string& current, ImGuiComboFlags flags)
{
    return BeginCombo(label, current.c_str(), flags);
}

bool IdStringCombo(const char* label, UnTech::idstring* value, const std::vector<UnTech::idstring>& list, bool includeBlank)
{
    return IdStringCombo(label, value, list, includeBlank,
                         [](const UnTech::idstring& name) { return &name; });
}

}
