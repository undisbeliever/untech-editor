/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../metasprite.h"
#include "../spriteimporter.h"
#include <memory>

namespace UnTech {
class ErrorList;

namespace MetaSprite {

std::unique_ptr<MetaSprite::FrameSet> utsi2utms(SpriteImporter::FrameSet& siFrameSet, ErrorList& errorList);

}
}
