/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "about-popup.h"
#include "message-box.h"
#include "gui/imgui-filebrowser.h"
#include "gui/imgui.h"
#include "gui/texture.h"
#include "gui/untech-editor.h"
#include "models/common/image.h"

namespace UnTech::Gui::AboutPopup {

static const char* const windowTitle = "About UnTech Editor";
static bool openOnNextFrame = false;

static Texture createLogoTexture()
{
    constexpr unsigned width = 68;
    constexpr unsigned height = 31;
    constexpr static std::array<char, width * height> pixels{
        "                                                                    "
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
        "              *******       **  **  **                              "
        "              **            **  **  **                              "
        "              **            **     *****                            "
        "              **        *** **  **  **   *****   ** **              "
        "              ******   **  ***  **  **  **   **  ***  *             "
        "              **      **    **  **  **  **   **  **                 "
        "              **      **    **  **  **  **   **  **                 "
        "              **      **    **  **  **  **   **  **                 "
        "              **       **  ***  **  **  **   **  **                 "
        "              *******   *** **  **  **   *****   **                 "
        "                                                                    "
        "                                                                   "
    };

    Image image(width, height);

    const rgba white(255, 255, 255, 255);
    const rgba transparent(0, 0, 0, 0);

    auto imgData = image.data();
    assert(imgData.size() == pixels.size());

    auto imgIt = imgData.begin();
    constexpr unsigned dataSize = width * height;

    static_assert(sizeof(pixels) == dataSize);
    for (const auto p : pixels) {
        *imgIt++ = p == '*' ? white : transparent;
    }
    assert(imgIt == imgData.end());

    return Texture::createFromImage(image);
}

static const Texture& logoTexture()
{
    static const Texture tex = createLogoTexture();
    return tex;
}

void openPopup()
{
    openOnNextFrame = true;
}

void processGui()
{
    using namespace std::string_literals;

    constexpr unsigned scale = 6;
    const ImVec2 buttonSize(96, 32);
    const float buttonSpacing = 32;

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
        ImGui::TextUnformatted("Copyright (c) 2016 - 2021, Marcus Rowe\n"
                               "\n"
                               "Licensed under The MIT License\n"
                               "https://github.com/undisbeliever/untech-editor/blob/master/LICENSE  \n\n"s);
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextUnformatted("\nThird Party Libraries:"s);
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
                               "Copyright (c) 2011-2020, Yann Collet\n"
                               "BSD 2-Clause License, https://github.com/lz4/lz4/blob/dev/lib/LICENSE  \n\n"s);
        ImGui::Unindent();

        ImGui::TextUnformatted("Dear ImGui"s);
        ImGui::Indent();
        ImGui::TextUnformatted("https://github.com/ocornut/imgui\n"
                               "Copyright (c) 2014-2021 Omar Cornut\n"
                               "MIT License, https://github.com/ocornut/imgui/blob/master/LICENSE.txt  \n\n"s);
        ImGui::Unindent();

        ImGui::TextUnformatted("imgui-filebrowser"s);
        ImGui::Indent();
        ImGui::TextUnformatted("https://github.com/AirGuanZ/imgui-filebrowser\n"
                               "Copyright (c) 2019-2020 Zhuang Guan\n"
                               "MIT License, https://github.com/AirGuanZ/imgui-filebrowser/blob/master/LICENSE  \n\n"s);
        ImGui::Unindent();

#ifdef IMGUI_IMPL_OPENGL_LOADER_GL3W
        ImGui::TextUnformatted("gl3w"s);
        ImGui::Indent();
        ImGui::TextUnformatted("https://github.com/skaslev/gl3w\n"
                               "Public Domain, https://github.com/skaslev/gl3w/blob/master/UNLICENSE  \n\n"s);
        ImGui::Unindent();

        ImGui::TextUnformatted("glcorearb.h"s);
        ImGui::Indent();
        ImGui::TextUnformatted("Copyright 2013-2020 The Khronos Group Inc.\n"
                               "MIT License  \n\n"s);
        ImGui::Unindent();
#endif

        ImGui::Unindent();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (UnTechEditor::instance()) {
            ImGui::SetCursorPosX((windowSize.x - buttonSize.x) / 2);
            if (ImGui::Button("Close", buttonSize)) {
                ImGui::CloseCurrentPopup();
            }
        }
        else {
            ImGui::SetCursorPosX((windowSize.x - buttonSize.x * 2 - buttonSpacing) / 2);

            if (auto fn = ImGui::SaveFileDialogButton("New Project", "New Project"s, ".utproject", buttonSize)) {
                UnTechEditor::newProject(*fn);
            }

            ImGui::SameLine(0, buttonSpacing);

            if (auto fn = ImGui::OpenFileDialogButton("Open Project", "Open Project"s, ".utproject", buttonSize)) {
                UnTechEditor::loadProject(*fn);
            }

            // Close about popup if the system has successfully opened/created a new project
            if (UnTechEditor::instance()) {
                ImGui::CloseCurrentPopup();
            }

            // Required if I want a MessageBox and the AboutPopop open at the same time.
            MsgBox::processGui();
        }

        ImGui::EndPopup();
    }

    // Putting this at the end allows me to call `AboutPopup::openPopup()` before the first `ImGui::NewFrame()` call
    if (openOnNextFrame) {
        ImGui::OpenPopup(windowTitle);
        openOnNextFrame = false;
    }
}

}
