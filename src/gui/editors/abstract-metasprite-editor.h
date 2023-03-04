/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/animation-timer.h"
#include "gui/selection.h"
#include "models/common/idstring.h"
#include "models/metasprite/animation/previewstate.h"

namespace UnTech::Gui {

class AbstractMetaSpriteEditorData : public AbstractExternalFileEditorData {
protected:
    friend class AbstractMetaSpriteEditorGui;

    SingleSelection animationsSel;
    MultipleChildSelection animationFramesSel;

    explicit AbstractMetaSpriteEditorData(ItemIndex itemIndex)
        : AbstractExternalFileEditorData(itemIndex)
    {
    }

    void updateSelection() override;
};

class AbstractMetaSpriteEditorGui : public AbstractEditorGui {
public:
    struct ExportOrderTree {
        idstring name;
        bool valid;
        std::vector<idstring> alternatives;
    };
    std::vector<ExportOrderTree> _eoStillFrames;
    std::vector<ExportOrderTree> _eoAnimations;

    AnimationTimer _animationTimer;
    UnTech::MetaSprite::Animation::PreviewState _animationState;
    unsigned prevAnimationIndex = 0;

    // MUST BE cleared when the export order changes or when the frame/animation list is resize, reordered or renamed.
    bool _exportOrderValid = false;

    bool _animationHFlip = false;
    bool _animationVFlip = false;

    bool showAnimationPreviewWindow = false;
    bool showExportOrderWindow = false;

    static bool showTileHitbox;
    static bool showShield;
    static bool showHitbox;
    static bool showHurtbox;

    static bool showFrameObjects;
    static bool showActionPoints;

public:
    void resetState() override;

    void viewMenu() override;

protected:
    AbstractMetaSpriteEditorGui(const char* strId)
        : AbstractEditorGui(strId)
    {
    }

    void showLayerButtons() const;
    void showExtraWindowButtons();

    template <typename AP, typename EditorT, typename FrameSetT>
    void animationPropertiesGui(const std::shared_ptr<EditorT>& editor, FrameSetT* frameSet);

    template <typename EditorT, typename DrawFunction>
    void animationPreviewWindow(const std::shared_ptr<EditorT>& data, DrawFunction drawFunction);

    void exportOrderWindow();

    template <typename FrameSetT>
    void updateExportOderTree(const FrameSetT& frameSet, const Project::ProjectFile& projectFile);

    virtual void addFrame(const idstring& name) = 0;
    virtual void addAnimation(const idstring& name) = 0;
};

void durationFormatText(const MetaSprite::Animation::DurationFormat df, uint8_t duration);

}
