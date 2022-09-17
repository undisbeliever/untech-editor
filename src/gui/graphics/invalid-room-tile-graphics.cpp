/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "invalid-room-tile-graphics.h"
#include "gui/imgui-drawing.h"
#include "gui/style.h"

namespace UnTech::Gui {

constexpr unsigned METATILE_SIZE_PX = 16;

void InvalidRoomTileGraphics::append(const Rooms::InvalidRoomTilesError& error)
{
    const auto oldSize = invalidTiles.size();
    invalidTiles.resize(oldSize + error.invalidTiles.size());

    auto it = invalidTiles.begin() + oldSize;

    for (const auto& tile : error.invalidTiles) {
        it->rect.x1 = tile.x * METATILE_SIZE_PX;
        it->rect.x2 = tile.x * METATILE_SIZE_PX + METATILE_SIZE_PX;
        it->rect.y1 = tile.y * METATILE_SIZE_PX;
        it->rect.y2 = tile.y * METATILE_SIZE_PX + METATILE_SIZE_PX;
        it->reasonBits = tile.reasonBits;
        it++;
    }
    assert(it == invalidTiles.end());
}

static void invalidRoomTileTooltip(const InvalidRoomTileGraphics::Tile& tile)
{
    using IRT = Rooms::InvalidRoomTile;

    if (tile.reasonBits == 0) {
        return;
    }

    ImGui::BeginTooltip();

    ImGui::Text("Invalid room tile (%d, %d)", tile.rect.x1 / METATILE_SIZE_PX, tile.rect.y1 / METATILE_SIZE_PX);
    ImGui::Spacing();

    static_assert(sizeof(tile.reasonBits) > 1);
    static_assert(std::is_same_v<decltype(tile.reasonBits), decltype(Rooms::InvalidRoomTile::reasonBits)>);

    if (tile.reasonBits & IRT::NO_EMPTY_TILE_ABOVE_DOWN_SLOPE) {
        ImGui::Bullet();
        ImGui::TextUnformatted(u8"Expected an empty tile above a down slope");
    }
    if (tile.reasonBits & IRT::NO_FLOOR_BELOW_DOWN_SLOPE) {
        ImGui::Bullet();
        ImGui::TextUnformatted(u8"Expected a floor tile below a down slope");
    }
    if (tile.reasonBits & IRT::NO_CEILING_ABOVE_UP_SLOPE) {
        ImGui::Bullet();
        ImGui::TextUnformatted(u8"Expected a ceiling tile above an up slope");
    }
    if (tile.reasonBits & IRT::NO_EMPTY_TILE_BELOW_UP_SLOPE) {
        ImGui::Bullet();
        ImGui::TextUnformatted(u8"Expected an empty tile below an up slope");
    }
    if (tile.reasonBits & IRT::INVALID_TILE_ON_THE_LEFT) {
        ImGui::Bullet();
        ImGui::TextUnformatted(u8"Invalid tile to the left");
    }
    if (tile.reasonBits & IRT::INVALID_TILE_ON_THE_RIGHT) {
        ImGui::Bullet();
        ImGui::TextUnformatted(u8"Invalid tile to the right");
    }
    if (tile.reasonBits & IRT::SLOPE_ON_BORDER) {
        ImGui::Bullet();
        ImGui::TextUnformatted(u8"Cannot have a slope tile on the room border");
    }

    ImGui::EndTooltip();
}

void InvalidRoomTileGraphics::draw(ImDrawList* drawList, const ImVec2& zoom, const ImVec2& screenOffset) const
{
    constexpr static float lineThickness = 2.0f;

    const ImVec2 mousePosFloat = ImGui::GetMousePos() - screenOffset;
    const point mousePos(std::floor(mousePosFloat.x / zoom.x), std::floor(mousePosFloat.y / zoom.y));

    for (const auto& tile : invalidTiles) {
        const ImVec2 p1(tile.rect.x1 * zoom.x + screenOffset.x, tile.rect.y1 * zoom.y + screenOffset.y);
        const ImVec2 p2(tile.rect.x2 * zoom.x + screenOffset.x, tile.rect.y2 * zoom.y + screenOffset.y);

        drawList->AddRectFilled(p1, p2, Style::invalidFillColor);
    }

    for (const auto& tile : invalidTiles) {
        const ImVec2 p1(tile.rect.x1 * zoom.x + screenOffset.x, tile.rect.y1 * zoom.y + screenOffset.y);
        const ImVec2 p2(tile.rect.x2 * zoom.x + screenOffset.x, tile.rect.y2 * zoom.y + screenOffset.y);

        drawList->AddRect(p1, p2, Style::invalidOutlineColor, 0.0, ImDrawFlags_RoundCornersNone, lineThickness);
    }

    if (ImGui::IsWindowHovered()) {
        for (const auto& tile : invalidTiles) {
            if (tile.rect.contains(mousePos)) {
                invalidRoomTileTooltip(tile);
            }
        }
    }
}

}
