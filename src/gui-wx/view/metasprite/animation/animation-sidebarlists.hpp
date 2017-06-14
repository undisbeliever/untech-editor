/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-wx/controllers/metasprite/animation.h"
#include "gui-wx/view/common/idmaplistctrl.h"
#include "gui-wx/view/common/vectorlistctrl.h"

namespace UnTech {
namespace View {

namespace MSA = UnTech::MetaSprite::Animation;

// ANIMATIONS
// ==========
template <>
void IdMapListCtrl<MSA::AnimationController>::CreateColumns();

template <>
void IdMapListCtrl<MSA::AnimationController>::CreateColumns()
{
    AppendColumn("Animation", wxLIST_FORMAT_LEFT);

    HideHeader();
    BindColumnsToEqualWidth();
}

template <>
wxString IdMapListCtrl<MSA::AnimationController>::OnGetItemText(long item, long column) const
{
    if (item < 0 || (unsigned)item >= _nameList.size()) {
        return wxEmptyString;
    }

    switch (column) {
    case 0: {
        return _nameList[item];
    }
    default:
        return wxEmptyString;
    }
}

// ANIMATION INSTRUCTIONS
// ======================
template <>
void VectorListCtrl<MSA::AnimationFrameController>::CreateColumns();

template <>
void VectorListCtrl<MSA::AnimationFrameController>::CreateColumns()
{
    AppendColumn("Frame", wxLIST_FORMAT_LEFT);
    AppendColumn("Duration", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();

    // Refresh duration column if Animation data changes
    _controller.parent().signal_dataChanged().connect([this](void) {
        this->Refresh();
    });
}

template <>
wxString VectorListCtrl<MSA::AnimationFrameController>::OnGetItemText(long item, long column) const
{
    const MSA::AnimationFrame::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const MSA::AnimationFrame& aFrame = list->at(item);
    const MSA::Animation& animation = _controller.parent().selected();

    switch (column) {
    case 0:
        return aFrame.frame.str();

    case 1:
        return animation.durationFormat.durationToString(aFrame.duration);

    default:
        return wxEmptyString;
    }
}
}
}
