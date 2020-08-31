/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech::Gui {

class Texture;

struct AboutPopup {
    static const char* const windowTitle;
    static bool openOnNextFrame;

    static void openPopup()
    {
        openOnNextFrame = true;
    }

    static void processGui();

private:
    static const Texture& logoTexture();
};

}
