/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imgui.h"
#include "shaders.h"
#include "untech-editor.h"
#include "gui/windows/about-popup.h"
#include "gui/windows/fps-window.h"
#include "gui/windows/message-box.h"
#include <iostream>

#if defined(IMGUI_IMPL_SDL_OPENGL)
#include "opengl/imgui_impl_opengl3.hpp"
#include "opengl/imgui_sdl_opengl3.hpp"
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define PLATFORM_WINDOWS
#endif

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

static std::filesystem::path executablePath()
{
#if defined(PLATFORM_WINDOWS)
    wchar_t path[MAX_PATH] = L"";
    const auto s = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (s == 0) {
        return {};
    }

    std::filesystem::path p(path);
#else
    Dl_info info;
    const auto r = dladdr((void*)&executablePath, &info);
    if (r == 0) {
        return {};
    }

    std::filesystem::path p(info.dli_fname);
#endif

    std::error_code ec;
    p = std::filesystem::canonical(p, ec);

    if (ec) {
        return {};
    }
    return p;
}

static void setIniFilename(ImGuiIO& io)
{
    static std::string iniFilename;

    io.IniFilename = nullptr;

    const auto p = executablePath();

    // Confirm we have the correct path
    if (p.filename().stem() == "untech-editor-gui") {
        const auto dir = p.parent_path();

        iniFilename = (dir / "untech-editor-gui.ini").string();

        io.IniFilename = iniFilename.c_str();
    }
}

static void setupGui(ImGuiIO& io)
{
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // This causes issues on my system - disabled for now
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    setIniFilename(io);

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

    Shaders::initialize();

    processProgramArguments(argc, argv);

    if (UnTechEditor::instance() == nullptr) {
        AboutPopup::openPopup();
    }

    while (true) {
        auto editor = UnTechEditor::instance();

        Shaders::newFrame();
        imgui.newFrame();

        centralDockSpace();

        if (editor) {
            editor->processGui();
        }

        AboutPopup::processGui();
        MessageBox::processGui();
        FpsWindow::processGui();

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
