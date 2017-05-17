/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/image.h"
#include <wx/bitmap.h>

namespace UnTech {
namespace View {

// Image SHOULD BE use premultiplied alpha
wxBitmap ImageToWxBitmap(const UnTech::Image& image);
}
}
