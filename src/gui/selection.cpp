/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "selection.h"
#include "imgui.h"
#include <cassert>
#include <climits>
#include <cstdint>

namespace UnTech::Gui {

void UpdateChildSelection(const SingleSelection* parent, MultipleChildSelection* sel)
{
    if (sel->parent != parent->selected) {
        sel->parent = parent->selected;
        sel->selected = MultipleSelection::NO_SELECTION;
        sel->clicked = UINT_MAX;
    }
}

void UpdateSelection(SingleSelection* sel)
{
    if (sel->clicked < SingleSelection::MAX_SIZE) {
        sel->selected = sel->clicked;
    }
    sel->clicked = SingleSelection::NO_SELECTION;
}

void UpdateSelection(MultipleSelection* sel)
{
    if (sel->clicked < int(MultipleSelection::MAX_SIZE)) {
        if (ImGui::GetIO().KeyCtrl) {
            sel->selected ^= 1 << sel->clicked;
        }
        else {
            sel->selected = 1 << sel->clicked;
        }
    }
    sel->clicked = UINT_MAX;
}

}
