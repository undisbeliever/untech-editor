#ifndef _UNTECH_GUI_COMMON_COLORTOGGLEBUTTON_H_
#define _UNTECH_GUI_COMMON_COLORTOGGLEBUTTON_H_

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {

// This method is hacky, since gtkmm does not have a colored rectangle
// widget it is the only one that works without using Gtk::DrawingArea

class ColorToggleButton : public Gtk::ToggleButton {
public:
    ColorToggleButton()
        : Gtk::ToggleButton("   ")
    {
    }

    void set_color(const UnTech::rgba& color)
    {
        Gdk::RGBA rgba;
        rgba.set_rgba(color.red / 255.0,
                      color.green / 255.0,
                      color.blue / 255.0,
                      color.alpha / 255.0);
        get_children()[0]->override_background_color(rgba);
    }

    void unset_color()
    {
        get_children()[0]->unset_background_color();
    }
};
}
}

#endif
