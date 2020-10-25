/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <string>

namespace UnTech::Gui {

namespace MessageBox {

void showMessage(const std::string& title, const std::string& message);
void showMessage(const std::string& title, const char* message);

void processGui();

}

}
