#ifndef _UNTECH_GUI_WIDGETS_COMMON_CR_RGBA_H_
#define _UNTECH_GUI_WIDGETS_COMMON_CR_RGBA_H_

#include <cairo.h>

struct cr_rgba {
    double red;
    double green;
    double blue;
    double alpha;

    inline void apply(const Cairo::RefPtr<Cairo::Context>& cr) const
    {
        cr->set_source_rgba(red, green, blue, alpha);
    }
};

#endif
