/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "background-image-editor.h"
#include "gui/aptable.h"
#include "gui/editor-actions.h"
#include "gui/imgui-drawing.h"
#include "gui/imgui-filebrowser.h"
#include "gui/style.h"
#include "models/project/project-data.h"

namespace UnTech::Gui {

// Background Image Action Policies
struct BackgroundImageEditorData::AP {
    struct BackgroundImage {
        using EditorT = BackgroundImageEditorData;
        using EditorDataT = UnTech::Resources::BackgroundImageInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            return namedListItem(&projectFile.backgroundImages, itemIndex.index);
        }
    };

    struct ImageFilename : public BackgroundImage {
        constexpr static auto FieldPtr = &EditorDataT::imageFilename;
        constexpr static auto validFlag = &BackgroundImageEditorGui::_textureValid;
    };
};

BackgroundImageEditorData::BackgroundImageEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
{
}

bool BackgroundImageEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    const auto* bi = namedListItem(&projectFile.backgroundImages, itemIndex().index);
    if (bi) {
        data = *bi;
    }
    return bi != nullptr;
}

void BackgroundImageEditorData::errorDoubleClicked(const AbstractError*)
{
    // There is no selection in BackgroundImageEditorData.
}

void BackgroundImageEditorData::updateSelection()
{
}

BackgroundImageEditorGui::BackgroundImageEditorGui()
    : AbstractEditorGui("##BG image editor")
    , _data(nullptr)
    , _imageTexture()
    , _textureValid(false)
{
}

bool BackgroundImageEditorGui::setEditorData(const std::shared_ptr<AbstractEditorData>& data)
{
    _data = std::dynamic_pointer_cast<BackgroundImageEditorData>(data);
    return _data != nullptr;
}

void BackgroundImageEditorGui::resetState()
{
    _textureValid = false;
}

void BackgroundImageEditorGui::editorClosed()
{
}

void BackgroundImageEditorGui::backgroundImageGui(const Project::ProjectFile& projectFile)
{
    using BackgroundImageInput = UnTech::Resources::BackgroundImageInput;

    assert(_data);
    auto& bi = _data->data;

    ImGui::TextUnformatted(u8"Background Image:");

    {
        if (Cell("Name", &bi.name)) {
            EditorActions<AP::BackgroundImage>::fieldEdited<
                &BackgroundImageInput::name>(_data);
        }

        if (Cell("Bit Depth", &bi.bitDepth)) {
            EditorActions<AP::BackgroundImage>::fieldEdited<
                &BackgroundImageInput::bitDepth>(_data);
        }

        if (ImGui::InputPngImageFilename("Image", &bi.imageFilename)) {
            EditorFieldActions<AP::ImageFilename>::fieldEdited(_data);
        }

        if (Cell("Conversion Palette", &bi.conversionPlette, projectFile.palettes)) {
            EditorActions<AP::BackgroundImage>::fieldEdited<
                &BackgroundImageInput::conversionPlette>(_data);
        }

        if (Cell("First Palette", &bi.firstPalette)) {
            EditorActions<AP::BackgroundImage>::fieldEdited<
                &BackgroundImageInput::firstPalette>(_data);
        }

        if (Cell("Number of Palettes", &bi.nPalettes)) {
            EditorActions<AP::BackgroundImage>::fieldEdited<
                &BackgroundImageInput::nPalettes>(_data);
        }

        if (Cell("Default Order", &bi.defaultOrder)) {
            EditorActions<AP::BackgroundImage>::fieldEdited<
                &BackgroundImageInput::defaultOrder>(_data);
        }
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    {
        // ::TODO read BackgroundImageData::tiles.size()::
        const unsigned numberOfTiles = 0;

        ImGui::Text("Number of Tiles: %u", numberOfTiles);
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ::TODO add palette animation::

    {
        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

        const ImVec2& zoom = Style::backgroundImageZoom.zoom();

        const ImVec2 imageSize(_imageTexture.width() * zoom.x, _imageTexture.height() * zoom.y);
        const ImVec2 screenOffset = captureMouseExpandCanvasAndCalcScreenPos("PaletteImage", imageSize);

        auto* drawList = ImGui::GetWindowDrawList();

        drawList->AddImage(_imageTexture.imguiTextureId(), screenOffset, screenOffset + imageSize);
        _invalidTiles.draw(drawList, zoom, screenOffset);

        Style::backgroundImageZoom.processMouseWheel();

        ImGui::EndChild();
    }
}

void BackgroundImageEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    updateImageTexture();

    backgroundImageGui(projectFile);
}

void BackgroundImageEditorGui::updateImageTexture()
{
    assert(_data);
    auto& bi = _data->data;

    if (_textureValid) {
        return;
    }

    // ::TODO draw tileset from projectData::

    _imageTexture.loadPngImage(bi.imageFilename);

    _textureValid = true;
}

void BackgroundImageEditorGui::resourceCompiled(const ErrorList& errors)
{
    using InvalidImageError = UnTech::Resources::InvalidImageError;

    _invalidTiles.clear();

    for (const auto& errorItem : errors.list()) {
        if (auto* imgErr = dynamic_cast<const InvalidImageError*>(errorItem.get())) {
            _invalidTiles.append(*imgErr);
        }
    }
}

}
