#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_TILESETGRAPHICALEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_TILESETGRAPHICALEDITOR_H_

#include "selection.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

template <class TilesetT>
class TilesetGraphicalEditor : public Gtk::Layout {
public:
    static constexpr unsigned TILE_SIZE = TilesetT::tile_t::TILE_SIZE;

public:
    TilesetGraphicalEditor(Selection& selection);

    ~TilesetGraphicalEditor() = default;

    void setZoom(double x, double y);

protected:
    // will always be called when a frameset is selected
    inline TilesetT& tileset() const;

    void redrawTilesetPixbuf();

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

    bool on_button_press_event(GdkEventButton* event) override;
    bool on_motion_notify_event(GdkEventMotion* event) override;
    bool on_button_release_event(GdkEventButton* event) override;

    bool on_enter_notify_event(GdkEventCrossing* event) override;
    bool on_leave_notify_event(GdkEventCrossing* event) override;

    void update_pointer_cursor();

    void setTilePixelForMouse(double x, double y);

private:
    Selection& _selection;

    double _zoomX, _zoomY;

    /**
     * _diaplayZoom is the global zoom used by Cairo.
     * It is set automatically depending on screen width
     * so that the borders of high-DPI displays are easily legible
     */
    double _displayZoom;

    /** A placeholder image to draw the tileset onto */
    UnTech::Image _tilesetImageBuffer;

    /** A pre-scaled copy of the tileset */
    Glib::RefPtr<Gdk::Pixbuf> _tilesetPixbuf;

    // If true then in draw tile mode
    bool _drawTileState;
};
}
}
}

#endif
