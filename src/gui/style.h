/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui.h"

namespace UnTech::Gui {

class Zoom {
private:
    unsigned _zoomInt;
    ImVec2 _zoom;
    std::string _zoomString;

public:
    Zoom(unsigned z);

    unsigned zoomInt() const { return _zoomInt; }
    const ImVec2& zoom() const { return _zoom; }
    const std::string& zoomString() const { return _zoomString; }

    void setZoom(unsigned z);

    void zoomCombo(const char* label);

    // Should be called just before `End()` or `EndChild()` to prevent jittering.
    void processMouseWheel();
};

struct Style {
    // Common
    constexpr static ImU32 gridColor = IM_COL32(128, 128, 128, 128);

    // MetaSprite / Sprite Importer
    constexpr static ImU32 frameObjectOutlineColor = IM_COL32(64, 128, 64, 240);
    constexpr static ImU32 actionPointOutlineColor = IM_COL32(192, 192, 192, 240);
    constexpr static ImU32 entityHitboxOutlineColor = IM_COL32(0, 0, 255, 240);
    constexpr static ImU32 tileHitboxOutlineColor = IM_COL32(192, 0, 0, 240);
    constexpr static ImU32 metaSpriteCrosshairColor = IM_COL32(128, 128, 128, 255);

    // Sprite Importer
    constexpr static ImU32 frameOutlineColor = IM_COL32(160, 160, 160, 240);
    constexpr static ImU32 antiHighlightColor = IM_COL32(128, 128, 128, 128);
    constexpr static ImU32 spriteImporterBackgroundColor = IM_COL32(192, 192, 192, 255);
    constexpr static ImU32 spriteImporterOriginColor = IM_COL32(128, 128, 128, 255);

    // MetaTiles
    constexpr static ImU32 tileCollisionTint = IM_COL32(192, 0, 192, 128);
    constexpr static ImU32 tileSelectionFillColor = IM_COL32(0, 0, 128, 128);
    constexpr static ImU32 tileSelectionOutlineColor = IM_COL32(0, 0, 255, 255);
    constexpr static ImU32 tileCursorInBoundsTint = IM_COL32(128, 255, 128, 255);
    constexpr static ImU32 tileCursorInBoundsOutline = IM_COL32(0, 128, 0, 255);
    constexpr static ImU32 tileCursorOutOfBoundsTint = IM_COL32(255, 128, 128, 255);
    constexpr static ImU32 tileCursorOutOfBoundsOutline = IM_COL32(128, 0, 0, 255);

    // MetaTile Properties
    static const ImVec4 tilePropertiesButtonTint;
    static const ImVec4 tilePropertiesButtonSelectedTint;

    // Room
    constexpr static ImU32 entityOutlineColor = IM_COL32(192, 0, 0, 240);
    constexpr static ImU32 entityFillColor = IM_COL32(192, 0, 0, 64);
    constexpr static ImU32 entranceOutlineColor = IM_COL32(0, 192, 0, 240);
    constexpr static ImU32 entranceFillColor = IM_COL32(0, 192, 0, 64);

    constexpr static ImU32 disabledEntityGroupTint = IM_COL32(255, 255, 255, 32);

    // Palette
    constexpr static ImU32 paletteRowLineColor = IM_COL32(200, 200, 255, 128);
    constexpr static ImU32 invalidFillColor = IM_COL32(255, 0, 0, 128);

    // Zoom
    static Zoom metaSpriteZoom;
    static Zoom spriteImporterZoom;
    static Zoom backgroundImageZoom;
    static Zoom metaTileTilesetZoom;
    static Zoom metaTileScratchpadZoom;
    static Zoom roomEditorZoom;
};

}
