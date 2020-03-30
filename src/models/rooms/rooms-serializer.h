/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "rooms.h"
#include "models/common/errorlist.h"
#include <filesystem>

namespace UnTech {
namespace Rooms {

// raises an exception on error
std::unique_ptr<RoomInput> loadRoomInput(const std::filesystem::path& filename);

// raises an exception on error
void saveRoomInput(const RoomInput& input, const std::filesystem::path& filename);

}
}
