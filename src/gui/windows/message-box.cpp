/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "message-box.h"
#include "gui/imgui.h"
#include "models/common/mutex_wrapper.h"
#include "models/common/u8strings.h"
#include <cmath>

using namespace std::string_literals;

// ::ANNOY cannot use MessageBox in mingw ::
// ::: "src/gui/windows/about-popup.cpp:185:13: error: ‘MessageBoxA’ is not a class, namespace, or enumeration" ::
namespace UnTech::Gui::MsgBox {

static const std::u8string dialogSuffix = u8"###MessageBox"s;

struct Message {
    std::u8string title;
    std::u8string message;

    Message(const std::u8string& t, std::u8string m)
        : title(t + dialogSuffix)
        , message(std::move(m))
    {
    }
};

struct MessageBoxState {
    std::vector<Message> messages{};
    bool toOpen = false;
};

static mutex<MessageBoxState> state;

void showMessage(const std::u8string& title, const std::u8string& message)
{
    state.access([&](auto& s) {
        s.messages.emplace_back(title, message);
        s.toOpen = true;
    });
}

void showMessage(const std::u8string& title, const char* message)
{
    state.access([&](auto& s) {
        s.messages.emplace_back(title, convert_old_string(message));
        s.toOpen = true;
    });
}

void processGui()
{
    state.access([&](auto& s) {
        if (s.messages.empty()) {
            return;
        }

        const auto& msg = s.messages.front();

        constexpr float maxTextWidth = 600;
        constexpr auto flags = ImGuiWindowFlags_NoResize;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 8));

        if (ImGui::BeginPopupModal(u8Cast(msg.title), nullptr, flags)) {
            const ImVec2 buttonSize(48, 24);
            const ImVec2 windowSize = ImGui::GetWindowSize();

            ImGui::PushTextWrapPos(maxTextWidth);
            ImGui::TextUnformatted(msg.message);
            ImGui::PopTextWrapPos();

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::SetCursorPosX(std::ceil((windowSize.x - buttonSize.x) / 2));
            if (ImGui::Button("OK", buttonSize)) {
                s.messages.erase(s.messages.begin());

                if (s.messages.empty()) {
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }

        if (s.toOpen) {
            ImGui::OpenPopup(u8Cast(dialogSuffix));
            s.toOpen = false;
        }

        ImGui::PopStyleVar();
    });
}

}
