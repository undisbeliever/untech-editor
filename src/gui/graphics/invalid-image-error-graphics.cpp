/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "invalid-image-error-graphics.h"
#include "gui/imgui-drawing.h"
#include "gui/style.h"

namespace UnTech::Gui {

void InvalidImageErrorGraphics::append(const Resources::InvalidImageError& imageErr)
{
    const auto oldSize = invalidTiles.size();
    invalidTiles.resize(oldSize + imageErr.invalidTiles.size());

    auto it = invalidTiles.begin() + oldSize;

    for (const auto& tile : imageErr.invalidTiles) {
        it->rect.x1 = tile.x;
        it->rect.x2 = tile.x + tile.size;
        it->rect.y1 = tile.y;
        it->rect.y2 = tile.y + tile.size;
        it->reason = tile.reason;
        it++;
    }
    assert(it == invalidTiles.end());
}

void InvalidImageErrorGraphics::draw(ImDrawList* drawList, const ImVec2& zoom, const ImVec2& screenOffset) const
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
                const char8_t* reason = Resources::InvalidImageError::reasonString(tile.reason);
                ImGui::ShowTooltipFmt("(%d, %d) %s", tile.rect.x1, tile.rect.y1, u8Cast(reason));
            }
        }
    }
}

}
