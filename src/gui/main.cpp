/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui.h"
#include "shaders.h"
#include "untech-editor.h"
#include "gui/windows/about-popup.h"
#include "gui/windows/message-box.h"
#include <iostream>

#if defined(IMGUI_IMPL_SDL_OPENGL)
#include "opengl/imgui_sdl_opengl3.hpp"
#endif

static void setupGui(ImGuiIO& io)
{
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Disable .ini settings file
    io.IniFilename = nullptr;

    // This causes issues on my system - disabled for now
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        // Tweak window style so platform and regular windows looks the same
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
void metricsWindow()
{
    static bool showMetrics = true;

    const auto& io = ImGui::GetIO();

    if (ImGui::IsKeyPressed(ImGuiKey_F12)) {
        if (io.KeyMods == ImGuiModFlags_None) {
            showMetrics = !showMetrics;
        }
    }
    if (showMetrics) {
        ImGui::ShowMetricsWindow(&showMetrics);
    }
}
#endif

static void processProgramArguments(int argc, const char* const argv[])
{
    using namespace UnTech::Gui;

    const std::string_view argument = argc > 1 ? argv[1] : "";
    if (argc > 2 || argument == "--help") {
        std::cout << "Usage " << argv[0] << " <filename>";
        exit(EXIT_SUCCESS);
    }
    else if (!argument.empty()) {
        UnTechEditor::loadProject(argument);
    }
}

int main(int argc, char* argv[])
{
    using namespace UnTech::Gui;

    ImGuiLoop imgui;
    imgui.init("UnTech Editor");

    ImGuiIO& io = ImGui::GetIO();
    setupGui(io);

    Shaders::initialize();

    processProgramArguments(argc, argv);

    if (UnTechEditor::instance() == nullptr) {
        AboutPopup::openPopup();
    }

    while (true) {
        auto editor = UnTechEditor::instance();

        Shaders::newFrame();
        imgui.newFrame();

        if (editor) {
            editor->processGui();
        }

        AboutPopup::processGui();
        MsgBox::processGui();

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
        metricsWindow();
#endif

        Shaders::processOffscreenRendering();
        imgui.render();

        if (editor) {
            editor->updateProjectFile();

            if (imgui.requestExitApplication) {
                editor->requestExitEditor();
                imgui.requestExitApplication = false;
            }
            if (editor->editorExited()) {
                break;
            }
        }
        else {
            if (imgui.requestExitApplication) {
                break;
            }
        }
    }

    // Close the project and stop the background thread to prevent a potential use-after-free error on cleanup.
    UnTechEditor::closeProject();

    Shaders::cleanup();
    imgui.cleanup();

    return EXIT_SUCCESS;
}
