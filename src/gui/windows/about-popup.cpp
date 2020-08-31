/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "about-popup.h"
#include "gui/imgui.h"
#include "gui/texture.h"
#include "models/common/image.h"

namespace UnTech::Gui {

const char* const AboutPopup::windowTitle = "About UnTech Editor";
bool AboutPopup::openOnNextFrame = false;

static Image logoImage()
{
    // ::TODO find a way to embed images into the source code::
    constexpr unsigned width = 68;
    constexpr unsigned height = 31;
    constexpr static const char pixels[]
        = "                                                                    "
          "                                                                    "
          "  ***     ***             ***********                               "
          "  ***     ***             ***********                               "
          "  ***     ***             ***********                               "
          "  ***     ***                 ***                                   "
          "  ***     ***  **      **     ***     ********    *****   **    **  "
          "  ***     ***  ***     **     ***     ********   *******  **    **  "
          "  ***     ***  ****    **     ***     **        ***   **  **    **  "
          "  ***     ***  *****   **     ***     **        **        **    **  "
          "  ***     ***  ** ***  **     ***     ********  **        ********  "
          "  ***     ***  **  *** **     ***     ********  **        ********  "
          "  ****   ****  **   *****     ***     **        **        **    **  "
          "  ***********  **    ****     ***     **        ***   **  **    **  "
          "   *********   **     ***     ***     ********   *******  **    **  "
          "    *******    **      **     ***     ********    *****   **    **  "
          "                                                                    "
          "                                                                    "
          "                                                                    "
          "                            ** **  **                               "
          "               ******       ** **  **                               "
          "               **           **     **                               "
          "               **       *** ** ** *****  *****  ** **               "
          "               **      **  *** **  **   **   ** ***  *              "
          "               ****** **    ** **  **   **   ** **                  "
          "               **     **    ** **  **   **   ** **                  "
          "               **     **    ** **  **   **   ** **                  "
          "               **      **  *** **  **   **   ** **                  "
          "               ******   *** ** **  **    *****  **                  "
          "                                                                    "
          "                                                                    ";

    Image image(width, height);

    const rgba white(255, 255, 255, 255);
    const rgba transparent(0, 0, 0, 0);

    rgba* imgData = image.data();
    const char* pixelData = pixels;
    constexpr unsigned dataSize = width * height;

    static_assert(IM_ARRAYSIZE(pixels) == dataSize + 1);
    for (unsigned i = 0; i < dataSize; i++) {
        *imgData++ = *pixelData++ == '*' ? white : transparent;
    }
    assert(imgData == image.data() + image.dataSize());

    return image;
}

const Texture& AboutPopup::logoTexture()
{
    static const Texture tex = Texture::createFromImage(logoImage());
    return tex;
}

void AboutPopup::processGui()
{
    using namespace std::string_literals;

    constexpr unsigned scale = 6;
    const ImVec2 buttonSize(96, 32);

    if (openOnNextFrame) {
        ImGui::OpenPopup(windowTitle);
        openOnNextFrame = false;
    }

    if (ImGui::BeginPopupModal(windowTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        const ImVec2 viewportSize = ImGui::GetWindowViewport()->Size;
        const ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::SetWindowPos(ImVec2((viewportSize.x - windowSize.x) / 2, (viewportSize.y - windowSize.y) / 2));

        {
            auto& texture = logoTexture();

            const ImVec4& tint = ImGui::GetStyleColorVec4(ImGuiCol_Text);

            const float offset = (windowSize.x - (texture.width() * scale)) / 2;
            if (offset > 0) {
                ImGui::SetCursorPosX(offset);
            }
            ImGui::Image(texture.imguiTextureId(), ImVec2(scale * texture.width(), scale * texture.height()),
                         ImVec2(0, 0), ImVec2(1, 1), tint);
        }

        ImGui::Spacing();

        ImGui::Indent();
        ImGui::TextUnformatted("Copyright (c) 2016 - 2020, Marcus Rowe\n"
                               "\n"
                               "Licensed under The MIT License\n"
                               "https://github.com/undisbeliever/untech-editor/blob/master/LICENSE  \n\n"s);
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextUnformatted("Third Party Libraries:"s);
        ImGui::Indent();

        ImGui::TextUnformatted("LodePNG"s);
        ImGui::Indent();
        ImGui::TextUnformatted("http://lodev.org/lodepng/\n"
                               "Copyright (c) 2005-2020 Lode Vandevenne\n"
                               "zlib License, https://github.com/lvandeve/lodepng/blob/master/LICENSE  \n\n");
        ImGui::Unindent();

        ImGui::TextUnformatted("LZ4 Library"s);
        ImGui::Indent();
        ImGui::TextUnformatted("https://lz4.github.io/lz4/\n"
                               "Copyright (c) 2011-2018, Yann Collet\n"
                               "BSD 2-Clause License, https://github.com/lz4/lz4/blob/master/lib/LICENSE  \n\n"s);
        ImGui::Unindent();

        ImGui::TextUnformatted("Dear ImGui"s);
        ImGui::Indent();
        ImGui::TextUnformatted("https://github.com/ocornut/imgui\n"
                               "Copyright (c) 2014-2020 Omar Cornut\n"
                               "MIT License, https://github.com/ocornut/imgui/blob/master/LICENSE.txt  \n\n"s);
        ImGui::Unindent();

        ImGui::Unindent();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SetCursorPosX((windowSize.x - buttonSize.x) / 2);
        if (ImGui::Button("Close", buttonSize)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

}
