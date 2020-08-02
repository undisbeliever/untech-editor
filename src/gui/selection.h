/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui.h"
#include <cassert>
#include <climits>
#include <cstdint>

namespace UnTech::Gui {

struct SingleSelection {
    constexpr static unsigned MAX_SIZE = UINT_MAX - 1;
    constexpr static unsigned NO_SELECTION = UINT_MAX;

    unsigned selected = NO_SELECTION;

    // The index of the `Selectable` that was pressed on this frame
    unsigned clicked = UINT_MAX;
};

struct MultipleSelection {
    constexpr static unsigned MAX_SIZE = 64;
    constexpr static uint64_t NO_SELECTION = 0;

    uint64_t selected = NO_SELECTION;

    // The index of the `Selectable` that was pressed on this frame
    unsigned clicked = UINT_MAX;
};

struct MultipleChildSelection : public MultipleSelection {
    unsigned parent = SingleSelection::NO_SELECTION;
};

// To be called before the child list is processed
void UpdateChildSelection(const SingleSelection* parent, MultipleChildSelection* sel);

// Done at the end of loop to prevent a graphical glitch
void UpdateSelection(SingleSelection* sel);

// Done at the end of loop to prevent a graphical glitch
void UpdateSelection(MultipleSelection* sel);

}
