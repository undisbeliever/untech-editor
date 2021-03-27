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

    AbstractMetaSpriteEditorData(ItemIndex itemIndex)
        : AbstractExternalFileEditorData(itemIndex)
    {
    }

    virtual void updateSelection() override;
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
    unsigned prevAnimationIndex;

    // MUST BE cleared when the export order changes or when the frame/animation list is resize, reordered or renamed.
    bool _exportOrderValid;

    bool _animationHFlip;
    bool _animationVFlip;

    static bool showFrameObjects;
    static bool showTileHitbox;
    static bool showEntityHitboxes;
    static bool showActionPoints;

public:
    virtual void resetState() override;

    virtual void viewMenu() override;

protected:
    AbstractMetaSpriteEditorGui() = default;

    void showLayerButtons() const;

    // Must be called in `setEditorData`
    template <typename EditorDataT>
    void setMetaSpriteData(EditorDataT* data);

    template <typename AP, typename EditorT, typename FrameSetT>
    void animationPropertiesWindow(const char* windowLabel, EditorT* editor, FrameSetT* frameSet);

    template <typename EditorDataT, typename DrawFunction>
    void animationPreviewWindow(const char* windowLabel, EditorDataT* data, DrawFunction drawFunction);

    void exportOrderWindow(const char* windowLabel);

    template <typename FrameSetT>
    void updateExportOderTree(const FrameSetT& frameSet, const Project::ProjectFile& projectFile);

    virtual void addFrame(const idstring& name) = 0;
    virtual void addAnimation(const idstring& name) = 0;
};

}
