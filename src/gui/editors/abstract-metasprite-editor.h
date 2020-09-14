/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/selection.h"

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
protected:
    AbstractMetaSpriteEditorGui() = default;

    template <typename AP, typename EditorT, typename FrameSetT>
    void animationPropertiesWindow(const char* windowLabel, EditorT* editor, FrameSetT* frameSet);

    template <typename AP, typename EditorT, typename FrameSetT>
    void animationPreviewWindow(const char* windowLabel, EditorT* editor, FrameSetT* frameSet);

    template <typename AP, typename EditorT, typename FrameSetT>
    void exportOrderWindow(const char* windowLabel, EditorT* editor, FrameSetT* frameSet);
};

}
