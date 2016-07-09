#pragma once

#include "gui/controllers/metasprite.h"
#include "gui/view/common/orderedlist.h"

namespace UnTech {
namespace View {

namespace MS = UnTech::MetaSprite;

template class OrderedListCtrl<MS::FrameObject>;

// FRAME OBJECTS
// =============

template <>
void OrderedListCtrl<MS::FrameObject>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
    AppendColumn("Size", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
}

template <>
wxString OrderedListCtrl<MS::FrameObject>::OnGetItemText(long item, long column) const
{
    const MS::FrameObject::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const MS::FrameObject& obj = list->at(item);

    switch (column) {
    case 0: {
        auto loc = obj.location();
        return wxString::Format("%d, %d", int(loc.x), int(loc.y));
    }

    case 1: {
        typedef UnTech::MetaSprite::FrameObject::ObjectSize OS;
        const static wxString smallString("Small");
        const static wxString largeString("Large");

        return obj.size() == OS::LARGE ? largeString : smallString;
    }

    default:
        return wxEmptyString;
    }
}

// ACTION POINTS
// =============

template <>
void OrderedListCtrl<MS::ActionPoint>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
    AppendColumn("Parameter", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
}

template <>
wxString OrderedListCtrl<MS::ActionPoint>::OnGetItemText(long item, long column) const
{
    const MS::ActionPoint::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const MS::ActionPoint& ap = list->at(item);

    switch (column) {
    case 0: {
        auto loc = ap.location();
        return wxString::Format("%d, %d", int(loc.x), int(loc.y));
    }

    case 1: {
        return wxString::Format("0x%02x", ap.parameter());
    }

    default:
        return wxEmptyString;
    }
}

// ENTITY HITBOXES
// ===============

template <>
void OrderedListCtrl<MS::EntityHitbox>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
    AppendColumn("Size", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
    AppendColumn("Parameter", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
}

template <>
wxString OrderedListCtrl<MS::EntityHitbox>::OnGetItemText(long item, long column) const
{
    const MS::EntityHitbox::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const MS::EntityHitbox& eh = list->at(item);

    switch (column) {
    case 0: {
        auto aabb = eh.aabb();
        return wxString::Format("%d, %d", int(aabb.x), int(aabb.y));
    }

    case 1: {
        auto aabb = eh.aabb();
        return wxString::Format("%d x %d", aabb.width, aabb.height);
    }

    case 2: {
        return wxString::Format("0x%02x", eh.parameter());
    }

    default:
        return wxEmptyString;
    }
}
}
}
