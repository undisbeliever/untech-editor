/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-metasprite-editor.h"
#include "gui/imgui-combos.h"
#include "gui/imgui-drawing.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "gui/selection.h"
#include "gui/style.h"
#include "models/common/clamp.h"
#include "models/common/iterators.h"
#include "models/metasprite/animation/animation.h"
#include "models/metasprite/animation/getters.h"
#include <cmath>

namespace UnTech::Gui {

inline void AbstractMetaSpriteEditorData::updateSelection()
{
    animationsSel.update();
    animationFramesSel.update(animationsSel);
}

template <typename AP, typename EditorT, typename FrameSetT>
void AbstractMetaSpriteEditorGui::animationPropertiesWindow(const char* windowLabel, EditorT* editor, FrameSetT* frameSet)
{
    using MsAnimation = UnTech::MetaSprite::Animation::Animation;

    ImGui::SetNextWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(windowLabel)) {

        ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.4f);

        ListButtons<typename AP::Animations_EditName>(editor);

        ImGui::SetNextItemWidth(-1);
        ImGui::NamedListListBox("##AnimationList", &editor->animationsSel, frameSet->animations);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (editor->animationsSel.selectedIndex() < frameSet->animations.size()) {
            MsAnimation& animation = frameSet->animations.at(editor->animationsSel.selectedIndex());

            {
                ImGui::InputIdstring("Name", &animation.name);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    ListActions<typename AP::Animations_EditName>::template selectedFieldEdited<
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

                ImGui::Columns(3);

                ImGui::Separator();
                ImGui::NextColumn();
                ImGui::Text("Frame");
                ImGui::NextColumn();
                ImGui::Text("Duration");
                ImGui::NextColumn();
                ImGui::Separator();

                for (auto [i, aFrame] : enumerate(animation.frames)) {
                    bool edited = false;

                    ImGui::PushID(i);

                    ImGui::Selectable(&editor->animationFramesSel, i);
                    ImGui::NextColumn();

                    ImGui::SetNextItemWidth(-1);
                    edited |= ImGui::IdStringCombo("##frame", &aFrame.frame.name, frameSet->frames);

                    ImGui::SetNextItemWidth(-1);
                    edited |= ImGui::FlipCombo("##flip", &aFrame.frame.hFlip, &aFrame.frame.vFlip);
                    ImGui::NextColumn();

                    ImGui::SetNextItemWidth(-1);
                    edited |= ImGui::InputUint8("##duration", &aFrame.duration, 0);

                    durationFormatText(animation.durationFormat, aFrame.duration);
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

template <typename EditorDataT, typename DrawFunction>
void AbstractMetaSpriteEditorGui::animationPreviewWindow(const char* windowLabel, EditorDataT* data, DrawFunction drawFunction)
{
    assert(data);
    const auto& fs = data->data;
    const NamedList<UnTech::MetaSprite::Animation::Animation>& animations = fs.animations;

    if (data->animationsSel.selectedIndex() != prevAnimationIndex) {
        prevAnimationIndex = data->animationsSel.selectedIndex();
        _animationState.animationIndex = prevAnimationIndex;
        _animationState.resetAnimation();
    }

    ImGui::SetNextWindowSize(ImVec2(650, 650), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(windowLabel)) {

        {
            ImGui::ToggledButtonWithTooltip("P##Play", &_animationTimer.active, "Play");
            ImGui::SameLine();

            if (ImGui::ButtonWithTooltip("N##Next", "Next Display Frame")) {
                _animationTimer.stop();
                _animationState.processDisplayFrame(animations);
            }
            ImGui::SameLine();

            if (ImGui::ButtonWithTooltip("S##Skip", "Skip Animation Frame")) {
                _animationTimer.stop();
                _animationState.nextAnimationFrame(animations);
            }
            ImGui::SameLine();

            if (ImGui::ButtonWithTooltip("R##Reset", "Reset")) {
                _animationState.resetAnimation();
                _animationState.animationIndex = data->animationsSel.selectedIndex();
            }
            ImGui::SameLine(0.0f, 12.0f);

            showLayerButtons();
            ImGui::SameLine();

            Style::metaSpriteAnimationZoom.zoomCombo("##zoom");
        }

        _animationTimer.process([&]() {
            _animationState.processDisplayFrame(animations);

            if (!_animationState.running) {
                _animationTimer.stop();
            }
        });

        ImGui::Separator();

        ImGui::Columns(2);
        {
            if (ImGui::BeginCombo("Region", _animationTimer.ntscRegion ? "NTSC" : "PAL")) {
                if (ImGui::Selectable("NTSC")) {
                    _animationTimer.ntscRegion = true;
                }
                if (ImGui::Selectable("PAL")) {
                    _animationTimer.ntscRegion = false;
                }
                ImGui::EndCombo();
            }

            ImGui::FlipCombo("Flip", &_animationHFlip, &_animationVFlip);

            ImGui::SingleSelectionNamedListCombo("Animation", &data->animationsSel, fs.animations, false);

            ImGui::SingleSelectionNamedListCombo("Override Next Animation", &_animationState.overrideNextAnimationIndex, fs.animations, true);

            using PreviewState = decltype(_animationState);
            constexpr float MAX_VELOCITY = 3.0f;
            constexpr float DIVISOR = 1 << PreviewState::FP_SHIFT;

            float velocityX = _animationState.velocityFP.x / DIVISOR;
            float velocityY = _animationState.velocityFP.y / DIVISOR;

            if (ImGui::SliderFloat("X Velocity", &velocityX, -MAX_VELOCITY, MAX_VELOCITY)) {
                _animationState.velocityFP.x = velocityX * DIVISOR;
            }
            if (ImGui::SliderFloat("Y Velocity", &velocityY, -MAX_VELOCITY, MAX_VELOCITY)) {
                _animationState.velocityFP.y = velocityY * DIVISOR;
            }
        }
        ImGui::NextColumn();

        const auto [ani, aniFrame] = MetaSprite::Animation::getAnimationAndFrame(animations, _animationState.animationIndex, _animationState.aFrameIndex);

        if (ani && aniFrame) {
            const auto& msFrame = aniFrame->frame;

            ImGui::Text("Display Frame: %u", _animationState.displayFrameCount);
            ImGui::Text("Animation Frame: %s.%u", u8Cast(ani->name), _animationState.aFrameIndex);
            ImGui::Text("Next Animation: %s", u8Cast(ani->nextAnimationText()));
            ImGui::Text("MetaSprite Frame: %s%s", u8Cast(msFrame.name), u8Cast(msFrame.flipStringSuffix().data()));
        }
        else {
            _animationTimer.stop();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        {
            ImGui::BeginChild("Animation", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);

            const ImVec2 zoom = Style::metaSpriteAnimationZoom.zoom();
            const ImVec2 region = ImGui::GetContentRegionAvail();

            const int maxX = region.x / 2 / zoom.x;
            const int maxY = region.y / 2 / zoom.y;

            bool posChanged = false;
            auto pos = _animationState.positionInt();
            if (pos.x < -maxX) {
                pos.x = maxX - 1;
                posChanged = true;
            }
            else if (pos.x > maxX) {
                pos.x = -maxX + 1;
                posChanged = true;
            }
            if (pos.y < -maxY) {
                pos.y = maxY - 1;
                posChanged = true;
            }
            else if (pos.y > maxY) {
                pos.y = -maxY + 1;
                posChanged = true;
            }

            const ImVec2 offset = ImGui::GetWindowPos();
            const ImVec2 centerOffset(offset.x + region.x / 2, offset.y + region.y / 2);
            ImVec2 drawPos(pos.x * zoom.x + centerOffset.x, pos.y * zoom.y + centerOffset.y);

            {
                constexpr int moveSizeInt = 8;

                const ImVec2 moveSize(moveSizeInt * zoom.x, moveSizeInt * zoom.y);

                ImGui::SetCursorScreenPos(ImVec2(drawPos.x - moveSize.x / 2, drawPos.y - moveSize.y / 2));
                ImGui::InvisibleButton("Mover", moveSize);

                if (ImGui::IsItemHovered()) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                }
                if (ImGui::IsItemActive()) {
                    if (ImGui::IsMouseDragging(0)) {
                        const ImVec2 mousePos = ImGui::GetMousePos() - centerOffset;
                        pos.x = clamp<int>((mousePos.x + zoom.x / 2) / zoom.x, -maxX + 1, maxX - 1);
                        pos.y = clamp<int>((mousePos.y + zoom.y / 2) / zoom.y, -maxY + 1, maxY - 1);

                        posChanged = true;
                    }
                }
            }

            if (posChanged) {
                _animationState.setPositionInt(pos);
                drawPos = ImVec2(pos.x * zoom.x + centerOffset.x, pos.y * zoom.y + centerOffset.y);
            }

            if (aniFrame) {
                if (const auto f = fs.frames.find(aniFrame->frame.name)) {

                    // ::TODO confirm flip is bit perfect against a SNES console::
                    ImVec2 z = zoom;
                    if (aniFrame->frame.hFlip ^ _animationHFlip) {
                        z.x = -z.x;
                    }
                    if (aniFrame->frame.vFlip ^ _animationVFlip) {
                        z.y = -z.y;
                    }

                    drawFunction(drawPos, z, *f);
                }
            }
            else {
                auto* drawList = ImGui::GetWindowDrawList();

                constexpr float l = 25;
                constexpr float thickness = 3.0f;

                drawList->AddLine(ImVec2(drawPos.x - l, drawPos.y - l), ImVec2(drawPos.x + l, drawPos.y + l), Style::failColor, thickness);
                drawList->AddLine(ImVec2(drawPos.x + l, drawPos.y - l), ImVec2(drawPos.x - l, drawPos.y + l), Style::failColor, thickness);
            }

            Style::metaSpriteAnimationZoom.processMouseWheel();

            ImGui::EndChild();
        }
    }
    ImGui::End();
}

template <typename T>
inline void buildExportOrderTree(std::vector<AbstractMetaSpriteEditorGui::ExportOrderTree>* tree,
                                 const NamedList<UnTech::MetaSprite::FrameSetExportOrder::ExportName>& exportNames,
                                 const NamedList<T>& data)
{
    assert(tree->empty());

    for (auto [i, en] : const_enumerate(exportNames)) {
        bool valid = bool(data.find(en.name));
        if (!valid) {
            valid = std::any_of(en.alternatives.cbegin(), en.alternatives.cend(),
                                [&](auto& alt) { return data.find(alt.name); });
        }

        std::vector<idstring> alternatives(en.alternatives.size());
        std::transform(en.alternatives.begin(), en.alternatives.end(),
                       alternatives.begin(),
                       [](const auto& alt) { return alt.name; });

        tree->push_back({ en.name, valid, std::move(alternatives) });
    }
}

template <typename FrameSetT>
void AbstractMetaSpriteEditorGui::updateExportOderTree(const FrameSetT& frameSet,
                                                       const Project::ProjectFile& projectFile)
{
    if (_exportOrderValid) {
        return;
    }

    _eoStillFrames.clear();
    _eoAnimations.clear();

    optional<const UnTech::MetaSprite::FrameSetExportOrder&> eo = projectFile.frameSetExportOrders.find(frameSet.exportOrder);
    if (eo) {
        buildExportOrderTree(&_eoStillFrames, eo->stillFrames, frameSet.frames);
        buildExportOrderTree(&_eoAnimations, eo->animations, frameSet.animations);
    }

    _exportOrderValid = true;
}

}
