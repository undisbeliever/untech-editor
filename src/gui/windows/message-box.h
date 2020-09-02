/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <string>

namespace UnTech::Gui {
class UnTechEditor;

class MessageBox {
private:
    static std::string title;
    static std::string message;
    static bool toOpen;

public:
    static void showMessage(const std::string& title, const std::string& message);
    static void showMessage(const std::string& title, const char* message);

    static void processGui();
};

}
