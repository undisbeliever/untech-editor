/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech::Gui {

struct FpsWindow {
    static const char* const windowTitle;
    static bool _open;

    static bool isOpen() { return _open; }
    static void open() { _open = true; }

    static void processGui();
};

}
