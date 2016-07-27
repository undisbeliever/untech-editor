#pragma once

#include "models/common/image.h"
#include <wx/bitmap.h>

namespace UnTech {
namespace View {

// Image SHOULD BE use premultiplied alpha
wxBitmap ImageToWxBitmap(const UnTech::Image& image);
}
}
