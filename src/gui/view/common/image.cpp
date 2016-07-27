#include "image.h"
#include <wx/rawbmp.h>

using namespace UnTech;

wxBitmap View::ImageToWxBitmap(const UnTech::Image& image)
{
    if (image.empty()) {
        return wxNullBitmap;
    }

    const auto isize = image.size();
    wxBitmap bitmap(isize.width, isize.height, 32);

    wxAlphaPixelData data(bitmap);
    if (!data) {
        return wxNullBitmap;
    }

    wxAlphaPixelData::Iterator p(data);

    for (unsigned y = 0; y < isize.height; y++) {
        const UnTech::rgba* i = image.scanline(y);
        wxAlphaPixelData::Iterator rowStart = p;

        for (unsigned x = 0; x < isize.width; x++) {

            p.Red() = i->red;
            p.Green() = i->green;
            p.Blue() = i->blue;
            p.Alpha() = i->alpha;

            p++;
            i++;
        }

        p = rowStart;
        p.OffsetY(data, 1);
    }

    return bitmap;
}
