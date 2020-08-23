/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/imgui.h"
#include "models/project/project.h"

namespace ImGui {

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

bool EntityHitboxTypeCombo(const char* label, UnTech::MetaSprite::EntityHitboxType* v);

}
