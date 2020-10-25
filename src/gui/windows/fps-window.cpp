/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "fps-window.h"
#include "gui/imgui.h"

namespace UnTech::Gui::FpsWindow {

// ::TODO add show FPS Window to View menu::
static bool open = true;

void processGui()
{
    if (!open) {
        return;
    }

    if (ImGui::Begin("FPS", &open)) {
        const auto& io = ImGui::GetIO();

        ImGui::Text("%.3f ms/frame (%.1f FPS)", float(io.DeltaTime * 1000.0f), float(1.0f / io.DeltaTime));
        ImGui::Text("Average %.3f ms/frame (%.1f FPS)", float(1000.0f / io.Framerate), float(io.Framerate));
    }
    ImGui::End();
}

}
