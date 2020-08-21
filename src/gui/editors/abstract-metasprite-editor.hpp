/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-metasprite-editor.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "gui/selection.h"
#include "models/metasprite/animation/animation.h"

namespace UnTech::Gui {

template <typename AP, typename EditorT, typename FrameSetT>
void AbstractMetaSpriteEditor::animationPropertiesWindow(const char* windowLabel, EditorT* editor, FrameSetT* frameSet)
{
    using MsAnimation = UnTech::MetaSprite::Animation::Animation;

    if (ImGui::Begin(windowLabel)) {
        ImGui::SetWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);

        ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.4f);

        ListButtons<typename AP::Animations>(editor);

        ImGui::SetNextItemWidth(-1);
        ImGui::NamedListListBox("##AnimationList", &_animationsSel, frameSet->animations, 8);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (_animationsSel.selectedIndex() < frameSet->animations.size()) {
            MsAnimation& animation = frameSet->animations.at(_animationsSel.selectedIndex());

            {
                ImGui::InputIdstring("Name", &animation.name);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    ListActions<typename AP::Animations>::template selectedFieldEdited<
                        &MsAnimation::name>(editor);
                }

                if (ImGui::EnumCombo("Duration Format", &animation.durationFormat)) {
                    ListActions<typename AP::Animations>::template selectedFieldEdited<
                        &MsAnimation::durationFormat>(editor);
                }

                if (ImGui::Checkbox("One Shot", &animation.oneShot)) {
                    ListActions<typename AP::Animations>::template selectedFieldEdited<
                        &MsAnimation::oneShot>(editor);
                }

                if (ImGui::IdStringCombo("Next Animation", &animation.nextAnimation, frameSet->animations)) {
                    ListActions<typename AP::Animations>::template selectedFieldEdited<
                        &MsAnimation::nextAnimation>(editor);
                }
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            {
                ImGui::PushID("AnimationFrames");

                ImGui::TextUnformatted("Animation Frames:");

                ListButtons<typename AP::AnimationFrames>(editor);

                ImGui::BeginChild("Scroll");

                // ::TODO shrink first column::
                ImGui::Columns(3);

                ImGui::Separator();
                ImGui::NextColumn();
                ImGui::Text("Frame");
                ImGui::NextColumn();
                ImGui::Text("Duration");
                ImGui::NextColumn();
                ImGui::Separator();

                for (unsigned i = 0; i < animation.frames.size(); i++) {
                    auto& aFrame = animation.frames.at(i);

                    bool edited = false;

                    ImGui::PushID(i);

                    ImGui::Selectable(&_animationFramesSel, i);
                    ImGui::NextColumn();

                    ImGui::SetNextItemWidth(-1);
                    edited |= ImGui::IdStringCombo("##frame", &aFrame.frame.name, frameSet->frames);

                    ImGui::SetNextItemWidth(-1);
                    edited |= ImGui::FlipCombo("##flip", &aFrame.frame.hFlip, &aFrame.frame.vFlip);
                    ImGui::NextColumn();

                    ImGui::SetNextItemWidth(-1);
                    edited |= ImGui::InputUint8("##duration", &aFrame.duration, 0);

                    ImGui::TextUnformatted(animation.durationFormat.durationToString(aFrame.duration));
                    ImGui::NextColumn();

                    if (edited) {
                        ListActions<typename AP::AnimationFrames>::selectedListItemEdited(editor, i);
                    }

                    ImGui::PopID();
                }

                ImGui::Columns(1);
                ImGui::EndChild();
                ImGui::PopID();
            }
        }
    }
    ImGui::End();
}

template <typename AP, typename EditorT, typename FrameSetT>
void AbstractMetaSpriteEditor::animationPreviewWindow(const char* windowLabel, EditorT*, FrameSetT*)
{
    if (ImGui::Begin(windowLabel)) {
        ImGui::SetWindowSize(ImVec2(650, 650), ImGuiCond_FirstUseEver);

        // ::TODO Animation Preview Window::
        ImGui::Text("::TODO Animation Preview::");
    }
    ImGui::End();
}

template <typename AP, typename EditorT, typename FrameSetT>
void AbstractMetaSpriteEditor::exportOrderWindow(const char* windowLabel, EditorT*, FrameSetT*)
{
    if (ImGui::Begin(windowLabel)) {
        ImGui::SetWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);

        // ::TODO Export Order Window::
        ImGui::Text("::TODO Export Order Tree::");
    }
    ImGui::End();
}

inline void AbstractMetaSpriteEditor::updateSelection()
{
    _animationsSel.update();
    _animationFramesSel.update(_animationsSel);
}

}
