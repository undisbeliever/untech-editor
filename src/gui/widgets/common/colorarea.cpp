#include "colorarea.h"

using namespace UnTech::Widgets;

ColorArea::ColorArea()
    : Gtk::DrawingArea()
    , _color(0)
{
}

ColorArea::ColorArea(const UnTech::rgba& color)
    : Gtk::DrawingArea()
    , _color(color)
{
}

void ColorArea::set_color(const UnTech::rgba& color)
{
    if (_color != color) {
        _color = color;
        queue_draw();
    }
}

bool ColorArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    cr->set_source_rgba(_color.red / 255.0,
                        _color.green / 255.0,
                        _color.blue / 255.0,
                        _color.alpha / 255.0);

    cr->paint();

    return true;
}
