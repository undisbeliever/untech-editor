/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/imgui.h"
#include "gui/selection.h"
#include "models/common/iterators.h"
#include "models/project/project.h"

namespace ImGui {

bool BeginCombo(const char* label, const std::u8string& current, ImGuiComboFlags flags);

bool IdStringCombo(const char* label, UnTech::idstring* value, const std::vector<UnTech::idstring>& list,
                   bool includeBlank = false);

template <class ListT, typename UnaryFunction>
bool IdStringComboSelection(UnTech::idstring* value, const ListT& list, bool includeBlank, UnaryFunction getter)
{
    bool changed = false;

    if (includeBlank) {
        const bool selected = value->isValid() == false;
        if (ImGui::Selectable("##Empty", selected)) {
            if (!selected) {
                *value = UnTech::idstring();
                changed = true;
            }
        }
    }

    for (auto [i, item] : enumerate(list)) {
        const UnTech::idstring* name = getter(item);

        if (name && name->isValid()) {
            ImGui::PushID(i);

            const bool selected = (*name == *value);
            if (ImGui::Selectable(u8Cast(*name), selected)) {
                if (!selected) {
                    *value = *name;
                    changed = true;
                }
            }

            ImGui::PopID();
        }
    }

    return changed;
}

template <class T>
bool IdStringComboSelection(UnTech::idstring* value, const UnTech::NamedList<T>& list, bool includeBlank)
{
    return IdStringComboSelection(value, list, includeBlank,
                                  [](const T& item) { return &item.name; });
}

template <class ListT, typename UnaryFunction>
inline bool IdStringCombo(const char* label, UnTech::idstring* value, const ListT& list, bool includeBlank,
                          UnaryFunction getter)
{
    bool changed = false;

    if (ImGui::BeginCombo(label, u8Cast(*value))) {
        changed = IdStringComboSelection(value, list, includeBlank, getter);
        ImGui::EndCombo();
    }

    return changed;
}

template <class T>
bool IdStringCombo(const char* label, UnTech::idstring* value, const UnTech::NamedList<T>& list,
                   bool includeBlank = false)
{
    return IdStringCombo(label, value, list, includeBlank,
                         [](const T& item) -> const UnTech::idstring* { return &item.name; });
}

template <class T>
bool IdStringCombo(const char* label, UnTech::idstring* value, const UnTech::ExternalFileList<T>& list,
                   bool includeBlank = false)
{
    return IdStringCombo(label, value, list, includeBlank,
                         [](const UnTech::ExternalFileItem<T>& efi) -> const UnTech::idstring* { return efi.value ? &efi.value->name : nullptr; });
}

template <class ListT>
bool SingleSelectionNamedListCombo(const char* label, unsigned* selectedIndexPtr, const ListT& list, bool includeBlank)
{
    bool entryClicked = false;
    auto& selectedIndex = *selectedIndexPtr;

    const char* const blankLabel = "------";

    const char* previewValue = blankLabel;
    if (selectedIndex < list.size()) {
        previewValue = u8Cast(list.at(selectedIndex).name);
    }

    if (ImGui::BeginCombo(label, previewValue)) {
        if (includeBlank) {
            const bool selected = selectedIndex >= list.size();
            if (ImGui::Selectable(blankLabel, selected)) {
                selectedIndex = INT_MAX;
                entryClicked = true;
            }
        }

        for (auto [i, item] : enumerate(list)) {
            const UnTech::idstring& name = item.name;

            ImGui::PushID(i);

            const bool selected = selectedIndex;
            if (ImGui::Selectable(u8Cast(name), selected)) {
                selectedIndex = i;
                entryClicked = true;
            }

            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    return entryClicked;
}

template <class ListT>
void SingleSelectionNamedListCombo(const char* label, UnTech::Gui::SingleSelection* sel, const ListT& list, bool includeBlank)
{
    const char* const blankLabel = "------";

    const char* previewValue = blankLabel;
    if (sel->selectedIndex() < list.size()) {
        previewValue = u8Cast(list.at(sel->selectedIndex()).name);
    }

    if (ImGui::BeginCombo(label, previewValue)) {
        if (includeBlank) {
            const bool selected = sel->selectedIndex() >= list.size();
            if (ImGui::Selectable(blankLabel, selected)) {
                sel->clearSelection();
            }
        }

        for (auto [i, item] : enumerate(list)) {
            const UnTech::idstring& name = item.name;

            ImGui::PushID(i);

            const bool selected = sel->isSelected(i);
            if (ImGui::Selectable(u8Cast(name), selected)) {
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
    int flips = (*vFlip << 1) | (*hFlip);
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
bool EnumCombo(const char* label, UnTech::Scripting::ArgumentType* v);

bool EnumCombo(const char* label, UnTech::Scripting::ConditionalType* v);
bool EnumCombo(const char* label, UnTech::Scripting::ComparisonType* v, UnTech::Scripting::ConditionalType t);

void TextEnum(const UnTech::Entity::DataType& type);
void TextEnum(const UnTech::Scripting::ArgumentType& v);

}
