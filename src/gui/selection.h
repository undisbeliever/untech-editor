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
#include <tuple>

namespace UnTech::Gui {

struct SingleSelection final {
    constexpr static unsigned MAX_SIZE = UINT_MAX - 1;
    constexpr static unsigned NO_SELECTION = UINT_MAX;

    unsigned selected = NO_SELECTION;

    // The index of the `Selectable` that was pressed on this frame
    unsigned clicked = UINT_MAX;

    std::tuple<> listArgs() const { return std::make_tuple(); }
};

struct MultipleSelection final {
    constexpr static unsigned MAX_SIZE = 64;
    constexpr static uint64_t NO_SELECTION = 0;

    uint64_t selected = NO_SELECTION;

    // The index of the `Selectable` that was pressed on this frame
    unsigned clicked = UINT_MAX;

    std::tuple<> listArgs() const { return std::make_tuple(); }
};

// Not a child class of MultipleSelection.
// Forces the use of parent in `UpdateSelection` and `ListButtons`
struct MultipleChildSelection final {
    constexpr static unsigned MAX_SIZE = 64;
    constexpr static uint64_t NO_SELECTION = 0;

    unsigned parent = SingleSelection::NO_SELECTION;
    uint64_t selected = NO_SELECTION;

    // The index of the `Selectable` that was pressed on this frame
    unsigned clicked = UINT_MAX;

    std::tuple<unsigned> listArgs() const { return { parent }; }
};

// Must be called at the end of processGui(), after the windows have been processed
void UpdateSelection(SingleSelection* sel);
void UpdateSelection(MultipleSelection* sel);
void UpdateSelection(MultipleChildSelection* sel, const SingleSelection& parent);

}
