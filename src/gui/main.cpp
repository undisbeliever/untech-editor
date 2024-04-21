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

    // Disable .ini settings file
    io.IniFilename = nullptr;

    // This causes issues on my system - disabled for now
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
}

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
void metricsWindow()
{
    static bool showMetrics = true;

    const auto& io = ImGui::GetIO();

    if (ImGui::IsKeyPressed(ImGuiKey_F12)) {
        if (io.KeyMods == ImGuiMod_None) {
            showMetrics = !showMetrics;
        }
    }
    if (showMetrics) {
        ImGui::ShowMetricsWindow(&showMetrics);
    }
}
#endif

static void processProgramArguments(const std::span<const char*> arguments)
{
    using namespace UnTech::Gui;

    assert(!arguments.empty());

    const std::string_view arg = arguments.size() > 1 ? arguments[1] : std::string_view();

    if (arguments.size() > 2 || arg == "--help") {
        std::cout << "Usage " << arguments.front() << " <filename>";
        exit(EXIT_SUCCESS);
    }
    else if (!arg.empty()) {
        UnTechEditor::loadProject(arg);
    }
}

static int run_main(std::span<const char*> arguments)
{
    using namespace UnTech::Gui;

    ImGuiLoop imgui;
    imgui.init("UnTech Editor");

    ImGuiIO& io = ImGui::GetIO();
    setupGui(io);

    Shaders::initialize();

    processProgramArguments(arguments);

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

int main(int argc, const char** const argv)
{
    try {
        if (argc < 1 || argv == nullptr) {
            std::cerr << "Invalid program arguments";
            return EXIT_FAILURE;
        }

        const auto arguments = std::span(argv, argc);

        const bool nullArgument = std::find(arguments.begin(), arguments.end(), nullptr) != arguments.end();
        if (nullArgument) {
            std::cerr << "Invalid program arguments";
            return EXIT_FAILURE;
        }

        return run_main(arguments);
    }
    catch (const std::exception& ex) {
        std::cerr << "UNCAUGHT EXCEPTION: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
