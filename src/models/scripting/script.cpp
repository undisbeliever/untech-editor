/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "script.h"

namespace UnTech::Scripting {

const idstring STARTUP_SCRIPT_NAME = u8"Startup"_id;

RoomScripts::RoomScripts()
{
    startupScript.name = STARTUP_SCRIPT_NAME;
}

}
