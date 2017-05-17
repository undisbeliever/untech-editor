/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/controllers/metasprite/metasprite.h"
#include "gui/view/common/idmaplistctrl.h"
#include "gui/view/common/vectorlistctrl.h"

namespace UnTech {
namespace View {

namespace MS = UnTech::MetaSprite::MetaSprite;

// FRAME
// =====
template <>
void IdMapListCtrl<MS::FrameController>::CreateColumns();

template <>
void IdMapListCtrl<MS::FrameController>::CreateColumns()
{
    AppendColumn("Frame", wxLIST_FORMAT_LEFT);

    HideHeader();
    BindColumnsToEqualWidth();
}

template <>
wxString IdMapListCtrl<MS::FrameController>::OnGetItemText(long item, long column) const
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

// FRAME OBJECTS
// =============
template <>
void VectorListCtrl<MS::FrameObjectController>::CreateColumns();

template <>
void VectorListCtrl<MS::FrameObjectController>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT);
    AppendColumn("Size", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();
}

template <>
wxString VectorListCtrl<MS::FrameObjectController>::OnGetItemText(long item, long column) const
{
    const MS::FrameObject::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const MS::FrameObject& obj = list->at(item);

    switch (column) {
    case 0: {
        return wxString::Format("%d, %d", int(obj.location.x), int(obj.location.y));
    }

    case 1: {
        using OS = UnTech::MetaSprite::ObjectSize;

        const static wxString smallString("Small");
        const static wxString largeString("Large");

        return obj.size == OS::LARGE ? largeString : smallString;
    }

    default:
        return wxEmptyString;
    }
}

// ACTION POINTS
// =============
template <>
void VectorListCtrl<MS::ActionPointController>::CreateColumns();

template <>
void VectorListCtrl<MS::ActionPointController>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT);
    AppendColumn("Parameter", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();
}

template <>
wxString VectorListCtrl<MS::ActionPointController>::OnGetItemText(long item, long column) const
{
    const MS::ActionPoint::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const MS::ActionPoint& ap = list->at(item);

    switch (column) {
    case 0: {
        return wxString::Format("%d, %d", int(ap.location.x), int(ap.location.y));
    }

    case 1: {
        return wxString::Format("0x%02x", int(ap.parameter));
    }

    default:
        return wxEmptyString;
    }
}

// ENTITY HITBOXES
// ===============
template <>
void VectorListCtrl<MS::EntityHitboxController>::CreateColumns();

template <>
void VectorListCtrl<MS::EntityHitboxController>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT);
    AppendColumn("Size", wxLIST_FORMAT_LEFT);
    AppendColumn("Type", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();
}

template <>
wxString VectorListCtrl<MS::EntityHitboxController>::OnGetItemText(long item, long column) const
{
    const MS::EntityHitbox::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const MS::EntityHitbox& eh = list->at(item);

    switch (column) {
    case 0: {
        return wxString::Format("%d, %d", int(eh.aabb.x), int(eh.aabb.y));
    }

    case 1: {
        return wxString::Format("%d x %d", int(eh.aabb.width), int(eh.aabb.height));
    }

    case 2: {
        return eh.hitboxType.string();
    }

    default:
        return wxEmptyString;
    }
}
}
}
