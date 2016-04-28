#ifndef _UNTECH_GUI_COMMON_COLORTOGGLEBUTTON_H_
#define _UNTECH_GUI_COMMON_COLORTOGGLEBUTTON_H_

#include "colorarea.h"
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {

class ColorToggleButton : public Gtk::ToggleButton {
public:
    ColorToggleButton()
        : Gtk::ToggleButton()
    {
        this->add(_area);
    }

    void set_color(const UnTech::rgba& color)
    {
        _area.set_color(color);
    }

    void unset_color()
    {
        _area.set_color(0);
    }

private:
    ColorArea _area;
};
}
}

#endif
