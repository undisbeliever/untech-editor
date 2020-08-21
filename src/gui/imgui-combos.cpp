/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui-combos.h"
#include "imgui.h"

namespace ImGui {

const std::array<const char*, 4> flipsComboItems = {
    "",
    "hFlip",
    "vFlip",
    "hvFlip"
};

// ::TODO move all enum combos to this file::

// ::TODO use enumMap for serialization and EnumCombo::
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
        for (unsigned i = 0; i < EHT::SHORT_STRING_VALUES.size(); i++) {
            if (ImGui::Selectable(EHT::SHORT_STRING_VALUES.at(i).c_str(), i == currentIndex)) {
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

}
