/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/imgui.h"
#include "gui/selection.h"
#include "models/project/project.h"

namespace ImGui {

template <class ListT>
void SingleSelectionNamedListCombo(const char* label, UnTech::Gui::SingleSelection* sel, const ListT& list, bool includeBlank)
{
    const char* const blankLabel = "------";

    const char* previewValue = blankLabel;
    if (sel->selectedIndex() < list.size()) {
        previewValue = list.at(sel->selectedIndex()).name.str().c_str();
    }

    if (ImGui::BeginCombo(label, previewValue)) {
        if (includeBlank) {
            const bool selected = sel->selectedIndex() >= list.size();
            if (ImGui::Selectable(blankLabel, selected)) {
                sel->clearSelection();
            }
        }
        for (unsigned i = 0; i < list.size(); i++) {
            const char* name = list.at(i).name.str().c_str();

            ImGui::PushID(i);

            const bool selected = sel->isSelected(i);
            if (ImGui::Selectable(name, selected)) {
                sel->setSelected(i);
            }

            ImGui::PopID();
        }

        ImGui::EndCombo();
    }
}

extern const std::array<const char*, 4> flipsComboItems;

inline bool FlipCombo(const char* label, bool* hFlip, bool* vFlip)
{
    int flips = (*vFlip << 1) | *hFlip;
    if (ImGui::Combo(label, &flips, flipsComboItems.data(), flipsComboItems.size())) {
        *hFlip = flips & 1;
        *vFlip = flips & 2;
        return true;
    }
    return false;
}

bool EnumCombo(const char* label, UnTech::MetaSprite::Animation::DurationFormat* v);
bool EnumCombo(const char* label, UnTech::MetaSprite::TilesetType* v);
bool EnumCombo(const char* label, UnTech::MetaSprite::ObjectSize* v);

bool EnumCombo(const char* label, UnTech::MetaSprite::SpriteImporter::UserSuppliedPalette::Position* v);
bool EnumCombo(const char* label, UnTech::Rooms::RoomEntranceOrientation* v);

bool EnumCombo(const char* label, UnTech::Entity::DataType* v);
bool EnumCombo(const char* label, UnTech::Entity::EntityType* v);
bool EnumCombo(const char* label, UnTech::Entity::ParameterType* v);

bool EnumCombo(const char* label, UnTech::Project::MappingMode* v);

bool EnumCombo(const char* label, UnTech::Resources::BgMode* v);
bool EnumCombo(const char* label, UnTech::Resources::LayerType* v);

bool EntityHitboxTypeCombo(const char* label, UnTech::MetaSprite::EntityHitboxType* v);

void TextEnum(const UnTech::Entity::DataType& type);

}
