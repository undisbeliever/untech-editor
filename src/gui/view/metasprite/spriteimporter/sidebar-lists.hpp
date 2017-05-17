/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/controllers/metasprite/spriteimporter.h"
#include "gui/view/common/idmaplistctrl.h"
#include "gui/view/common/vectorlistctrl.h"

namespace UnTech {
namespace View {

namespace SI = UnTech::MetaSprite::SpriteImporter;

// FRAME
// =====
template <>
void IdMapListCtrl<SI::FrameController>::CreateColumns();

template <>
void IdMapListCtrl<SI::FrameController>::CreateColumns()
{
    AppendColumn("Frame", wxLIST_FORMAT_LEFT);

    HideHeader();
    BindColumnsToEqualWidth();
}

template <>
wxString IdMapListCtrl<SI::FrameController>::OnGetItemText(long item, long column) const
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
void VectorListCtrl<SI::FrameObjectController>::CreateColumns();

template <>
void VectorListCtrl<SI::FrameObjectController>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT);
    AppendColumn("Size", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();
}

template <>
wxString VectorListCtrl<SI::FrameObjectController>::OnGetItemText(long item, long column) const
{
    const list_type* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const SI::FrameObject& obj = list->at(item);

    switch (column) {
    case 0: {
        return wxString::Format("%d, %d", obj.location.x, obj.location.y);
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
void VectorListCtrl<SI::ActionPointController>::CreateColumns();

template <>
void VectorListCtrl<SI::ActionPointController>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT);
    AppendColumn("Parameter", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();
}

template <>
wxString VectorListCtrl<SI::ActionPointController>::OnGetItemText(long item, long column) const
{
    const SI::ActionPoint::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const SI::ActionPoint& ap = list->at(item);

    switch (column) {
    case 0: {
        return wxString::Format("%d, %d", ap.location.x, ap.location.y);
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
void VectorListCtrl<SI::EntityHitboxController>::CreateColumns();

template <>
void VectorListCtrl<SI::EntityHitboxController>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT);
    AppendColumn("Size", wxLIST_FORMAT_LEFT);
    AppendColumn("Type", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();
}

template <>
wxString VectorListCtrl<SI::EntityHitboxController>::OnGetItemText(long item, long column) const
{
    const SI::EntityHitbox::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const SI::EntityHitbox& eh = list->at(item);

    switch (column) {
    case 0: {
        return wxString::Format("%d, %d", eh.aabb.x, eh.aabb.y);
    }

    case 1: {
        return wxString::Format("%d x %d", eh.aabb.width, eh.aabb.height);
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
