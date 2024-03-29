/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palette-editor.h"
#include "gui/aptable.h"
#include "gui/editor-actions.h"
#include "gui/imgui-drawing.h"
#include "gui/imgui-filebrowser.h"
#include "gui/style.h"
#include "models/common/clamp.h"
#include "models/common/iterators.h"

namespace UnTech::Gui {

// PaletteEditor Action Policies
struct PaletteEditorData::AP {
    struct Palette {
        using EditorT = PaletteEditorData;
        using EditorDataT = UnTech::Resources::PaletteInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            return namedListItem(&projectFile.palettes, itemIndex.index);
        }
    };

    struct ImageFilename : public Palette {
        constexpr static auto FieldPtr = &EditorDataT::paletteImageFilename;
        constexpr static auto validFlag = &PaletteEditorGui::_textureValid;
    };
};

PaletteEditorData::PaletteEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
{
}

bool PaletteEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    const auto* p = namedListItem(&projectFile.palettes, itemIndex().index);
    if (p) {
        data = *p;
    }
    return p != nullptr;
}

void PaletteEditorData::errorDoubleClicked(const AbstractError*)
{
    // There is no selection in PaletteEditorData.
}

void PaletteEditorData::updateSelection()
{
}

PaletteEditorGui::PaletteEditorGui()
    : AbstractEditorGui("##Palette editor")
    , _data(nullptr)
    , _animationTimer()
    , _imageTexture()
    , _frameId(0)
    , _textureValid(false)
{
}

bool PaletteEditorGui::setEditorData(const std::shared_ptr<AbstractEditorData>& data)
{
    _data = std::dynamic_pointer_cast<PaletteEditorData>(data);
    return _data != nullptr;
}

void PaletteEditorGui::resetState()
{
    _animationTimer.reset();
    _textureValid = false;
    _frameId = -1;
}

void PaletteEditorGui::editorClosed()
{
}

void PaletteEditorGui::paletteGui()
{
    using namespace std::string_literals;
    using PaletteInput = UnTech::Resources::PaletteInput;

    assert(_data);
    auto& palette = _data->data;

    const std::u8string windowName = palette.name + u8" Palette###Palette"s;

    {
        if (Cell("Name", &palette.name)) {
            EditorActions<AP::Palette>::fieldEdited<
                &PaletteInput::name>(_data);
        }

        if (ImGui::InputPngImageFilename("Image", &palette.paletteImageFilename)) {
            EditorFieldActions<AP::ImageFilename>::fieldEdited(_data);
        }

        if (Cell("Rows Per Frame", &palette.rowsPerFrame)) {
            EditorActions<AP::Palette>::fieldEdited<
                &PaletteInput::rowsPerFrame>(_data);
        }

        if (Cell("Animation Delay", &palette.animationDelay)) {
            EditorActions<AP::Palette>::fieldEdited<
                &PaletteInput::animationDelay>(_data);
        }

        if (Cell("Skip First Frame", &palette.skipFirstFrame)) {
            EditorActions<AP::Palette>::fieldEdited<
                &PaletteInput::skipFirstFrame>(_data);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const auto rowsPerFrame = clamp<unsigned>(palette.rowsPerFrame, 1, _imageTexture.height());
    const unsigned firstFrame = palette.skipFirstFrame ? 1 : 0;
    const unsigned nFrames = std::max<unsigned>(1, _imageTexture.height() / rowsPerFrame - firstFrame);

    {
        if (ImGui::ToggledButton("Play", _animationTimer.isActive())) {
            _animationTimer.playPause();
            if (_frameId < 0) {
                _frameId = 0;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            _animationTimer.reset();
            _frameId = -1;
        }
        ImGui::SameLine();
        if (ImGui::Button("Next Frame")) {
            _animationTimer.stop();
            _frameId++;
        }

        _animationTimer.process(palette.animationDelay, [&]() { _frameId++; });

        if (_frameId >= 0 && unsigned(_frameId) >= nFrames) {
            _frameId = 0;
        }
        ImGui::SameLine();
        if (_frameId >= 0) {
            ImGui::Text("Frame %d / %d", int(_frameId + 1), nFrames);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    {
        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

        if (_frameId < 0) {
            constexpr float lineWidth = 2.0f;
            constexpr float zoom = 24;

            const ImVec2 imageSize(_imageTexture.width() * zoom, _imageTexture.height() * zoom);
            const ImVec2 screenOffset = captureMouseExpandCanvasAndCalcScreenPos("Image", imageSize);

            auto* drawList = ImGui::GetWindowDrawList();

            drawList->AddImage(_imageTexture.imguiTextureId(), screenOffset, screenOffset + imageSize);

            const float x1 = screenOffset.x - zoom;
            const float x2 = x1 + imageSize.x + 2 * zoom;
            float y = screenOffset.y;
            const float yStep = rowsPerFrame * zoom;

            for ([[maybe_unused]] const auto i : range(nFrames)) {
                drawList->AddLine(ImVec2(x1, y), ImVec2(x2, y), Style::paletteRowLineColor, lineWidth);
                y += yStep;
            }
            drawList->AddLine(ImVec2(x1, y), ImVec2(x2, y), Style::paletteRowLineColor, lineWidth);

            if ((firstFrame + nFrames) * rowsPerFrame != _imageTexture.height()) {
                drawList->AddRectFilled(ImVec2(x1, y + lineWidth / 2), ImVec2(x2, screenOffset.y + imageSize.y), Style::invalidFillColor);
            }
        }
        else {
            constexpr float zoom = 48;

            const ImVec2 imageSize(_imageTexture.width() * zoom, rowsPerFrame * zoom);
            const ImVec2 screenOffset = captureMouseExpandCanvasAndCalcScreenPos("FrameImage", imageSize);

            auto* drawList = ImGui::GetWindowDrawList();

            const ImVec2 uvMin(0.0f, float((_frameId + firstFrame) * rowsPerFrame) / _imageTexture.height());
            const ImVec2 uvMax(1.0f, float((_frameId + firstFrame + 1) * rowsPerFrame) / _imageTexture.height());

            drawList->AddImage(_imageTexture.imguiTextureId(), screenOffset, screenOffset + imageSize, uvMin, uvMax);
        }

        ImGui::EndChild();
    }
}

void PaletteEditorGui::processGui(const Project::ProjectFile&, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    updateImageTexture();

    paletteGui();
}

void PaletteEditorGui::updateImageTexture()
{
    assert(_data);
    auto& palette = _data->data;

    if (_textureValid) {
        return;
    }

    _imageTexture.loadPngImage(palette.paletteImageFilename);

    _textureValid = true;
}

}
