/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "two-point-rect.h"
#include "gui/imgui-drawing.h"
#include "models/common/idstring.h"
#include "models/common/image.h"

namespace UnTech::Gui {

struct DrawEntitySettings {
    TwoPointRect hitboxRect;
    TwoPointRect imageRect;
    ImVec2 uvMin;
    ImVec2 uvMax;
};

class EntityGraphics {
private:
    Image _image;
    DrawEntitySettings _nullSetting;

public:
    EntityGraphics();

    const Image& image() const { return _image; }

    const DrawEntitySettings& settingsForPlayer(unsigned /* playerId */) const
    {
        return _nullSetting;
    }

    const DrawEntitySettings& settingsForEntity(const idstring&) const
    {
        return _nullSetting;
    }
};

}
