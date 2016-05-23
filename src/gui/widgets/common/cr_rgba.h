#pragma once

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
