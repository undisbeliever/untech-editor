/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui.h"
#include "untech-editor.h"
#include "gui/windows/about-popup.h"
#include "gui/windows/fps-window.h"
#include "gui/windows/message-box.h"
#include <iostream>

#if defined(IMGUI_IMPL_SDL_OPENGL)
#include "opengl/imgui_sdl_opengl3.hpp"
#endif

static void setupGui(ImGuiIO& io)
{
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // This causes issues on my system - disabled for now
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    // Disable window state ini loading/saving
    // ::TODO re-enable this in the future (maybe?)::
    io.IniFilename = NULL;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        // Tweak window style so platform and regular windows looks the same
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

static void centralDockSpace()
{
    const ImGuiWindowFlags windowFlags
        = ImGuiWindowFlags_NoDocking
          | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
          | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetWorkPos());
    ImGui::SetNextWindowSize(viewport->GetWorkSize());
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("UnTech Editor", nullptr, windowFlags);

    ImGui::PopStyleVar(3);

    const ImGuiID dockspaceID = ImGui::GetID("CentralDock");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f));

    ImGui::End();
}

static void processProgramArguments(int argc, const char* argv[])
{
    using namespace std::string_literals;
    using namespace UnTech::Gui;

    const std::string argument = argc > 1 ? argv[1] : "";
    if (argc > 2 || argument == "--help"s) {
        std::cout << "Usage " << argv[0] << " <filename>";
        exit(EXIT_SUCCESS);
    }
    else if (!argument.empty()) {
        UnTechEditor::loadProject(argument);
    }
}

int main(int argc, const char* argv[])
{
    using namespace UnTech::Gui;

    ImGuiLoop imgui;
    imgui.init("UnTech Editor");

    ImGuiIO& io = ImGui::GetIO();
    setupGui(io);

    processProgramArguments(argc, argv);

    if (UnTechEditor::instance() == nullptr) {
        AboutPopup::openPopup();
    }

    while (true) {
        auto editor = UnTechEditor::instance();

        imgui.newFrame();

        centralDockSpace();

        if (editor) {
            editor->processGui();
        }

        AboutPopup::processGui();
        MessageBox::processGui();
        FpsWindow::processGui();

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

    imgui.cleanup();

    return EXIT_SUCCESS;
}
