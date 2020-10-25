/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "message-box.h"
#include "gui/imgui.h"
#include <cmath>
#include <mutex>

namespace UnTech::Gui::MessageBox {

static const std::string dialogSuffix = "###MessageBox";

static std::mutex mutex;

static std::string title;
static std::string message;
static bool toOpen = false;

void showMessage(const std::string& t, const std::string& m)
{
    std::lock_guard lock(mutex);

    title = t + dialogSuffix;
    message = m;
    toOpen = true;
}

void showMessage(const std::string& t, const char* m)
{
    std::lock_guard lock(mutex);

    title = t + dialogSuffix;
    message = m;
    toOpen = true;
}

void processGui()
{
    std::lock_guard lock(mutex);

    if (toOpen) {
        ImGui::OpenPopup(dialogSuffix.c_str());
        toOpen = false;
    }

    constexpr float maxTextWidth = 600;
    constexpr auto flags = ImGuiWindowFlags_NoResize;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 8));

    if (ImGui::BeginPopupModal(title.c_str(), nullptr, flags)) {
        const ImVec2 buttonSize(48, 24);
        const ImVec2 windowSize = ImGui::GetWindowSize();

        ImGui::PushTextWrapPos(maxTextWidth);
        ImGui::TextUnformatted(message);
        ImGui::PopTextWrapPos();

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::SetCursorPosX(std::ceil((windowSize.x - buttonSize.x) / 2));
        if (ImGui::Button("OK", buttonSize)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
}

}
