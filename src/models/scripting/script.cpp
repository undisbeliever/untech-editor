/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "script.h"

namespace UnTech::Scripting {

using namespace std::string_literals;

const idstring STARTUP_SCRIPT_NAME = idstring::fromString("Startup");

RoomScripts::RoomScripts()
{
    startupScript.name = STARTUP_SCRIPT_NAME;
}

}
