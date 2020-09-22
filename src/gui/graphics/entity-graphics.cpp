/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entity-graphics.h"

// ::TODO move missingImageSymbol elsewhere::
#include "gui/texture.h"

namespace UnTech::Gui {

EntityGraphics::EntityGraphics()
    : _image(Texture::missingImageSymbol)
    , _nullSetting()
{
    _nullSetting.imageRect = TwoPointRect(-8, 8, -8, 8);
    _nullSetting.hitboxRect = TwoPointRect(-4, 4, -4, 4);
    _nullSetting.uvMin = ImVec2(0, 0);
    _nullSetting.uvMax = ImVec2(1, 1);
}

// ::TODO generate entity graphics::
// ::TODO generate entity graphics in a background thread::

}
