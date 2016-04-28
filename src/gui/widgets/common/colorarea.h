#ifndef _UNTECH_GUI_WIDGETS_COMMON_COLOR_AREA_H_
#define _UNTECH_GUI_WIDGETS_COMMON_COLOR_AREA_H_

#include "models/common/rgba.h"

#include <gtkmm/drawingarea.h>

namespace UnTech {
namespace Widgets {

class ColorArea : public Gtk::DrawingArea {
public:
    ColorArea();
    ColorArea(const UnTech::rgba& color);

    virtual ~ColorArea() = default;

    const UnTech::rgba& color() const { return _color; }

    void set_color(const UnTech::rgba& color);

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

private:
    UnTech::rgba _color;
};
}
}

#endif
