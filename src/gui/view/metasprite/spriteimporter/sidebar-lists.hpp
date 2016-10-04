#pragma once

#include "gui/controllers/sprite-importer.h"
#include "gui/view/common/namedlist.h"
#include "gui/view/common/orderedlist.h"

namespace UnTech {
namespace View {

namespace SI = UnTech::SpriteImporter;

// FRAME
// =====
template <>
void NamedListCtrl<SI::Frame>::CreateColumns();

template <>
void NamedListCtrl<SI::Frame>::CreateColumns()
{
    AppendColumn("Frame", wxLIST_FORMAT_LEFT);

    HideHeader();
    BindColumnsToEqualWidth();
}

template <>
wxString NamedListCtrl<SI::Frame>::OnGetItemText(long item, long column) const
{
    if (item < 0 || (unsigned)item >= _ptrList.size()) {
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
void OrderedListCtrl<SI::FrameObject>::CreateColumns();

template <>
void OrderedListCtrl<SI::FrameObject>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT);
    AppendColumn("Size", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();
}

template <>
wxString OrderedListCtrl<SI::FrameObject>::OnGetItemText(long item, long column) const
{
    const SI::FrameObject::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const SI::FrameObject& obj = list->at(item);

    switch (column) {
    case 0: {
        auto loc = obj.location();
        return wxString::Format("%d, %d", int(loc.x), int(loc.y));
    }

    case 1: {
        typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;
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
void OrderedListCtrl<SI::ActionPoint>::CreateColumns();

template <>
void OrderedListCtrl<SI::ActionPoint>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT);
    AppendColumn("Parameter", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();
}

template <>
wxString OrderedListCtrl<SI::ActionPoint>::OnGetItemText(long item, long column) const
{
    const SI::ActionPoint::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const SI::ActionPoint& ap = list->at(item);

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
void OrderedListCtrl<SI::EntityHitbox>::CreateColumns();

template <>
void OrderedListCtrl<SI::EntityHitbox>::CreateColumns()
{
    AppendColumn("Location", wxLIST_FORMAT_LEFT);
    AppendColumn("Size", wxLIST_FORMAT_LEFT);
    AppendColumn("Type", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();
}

template <>
wxString OrderedListCtrl<SI::EntityHitbox>::OnGetItemText(long item, long column) const
{
    const SI::EntityHitbox::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const SI::EntityHitbox& eh = list->at(item);

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
        return eh.hitboxType().string();
    }

    default:
        return wxEmptyString;
    }
}
}
}
