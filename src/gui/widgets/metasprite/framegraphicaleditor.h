#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_FRAMEGRAPHICALEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_FRAMEGRAPHICALEDITOR_H_

#include "selection.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class FrameGraphicalEditor : public Gtk::DrawingArea {
public:
    FrameGraphicalEditor(Selection& selection);

    ~FrameGraphicalEditor() = default;

    void setFrame(std::shared_ptr<MS::Frame> frame);

    void setZoom(double x, double y);

    void setCenter(int x, int y);

protected:
    struct Action {
        enum State {
            NONE = 0,
            CLICK,
            DRAG
        };

        State state = NONE;
        ms8point pressLocation;
        ms8point previousLocation;

        ms8rect dragAabb;
        bool canDrag;
        bool resize;
        bool resizeLeft;
        bool resizeRight;
        bool resizeTop;
        bool resizeBottom;
    };

    void redrawFramePixbuf();

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

    bool on_button_press_event(GdkEventButton* event) override;
    bool on_motion_notify_event(GdkEventMotion* event) override;
    bool on_button_release_event(GdkEventButton* event) override;

    void handleRelease_Click(const ms8point& mouse);
    void handleRelease_SelectTransparentColor(const ms8point& mouse);
    void handleRelease_Drag();

    bool on_enter_notify_event(GdkEventCrossing* event) override;
    bool on_leave_notify_event(GdkEventCrossing* event) override;

    void update_pointer_cursor();

    void update_offsets();

private:
    double _zoomX, _zoomY;
    int _xOffset, _yOffset;

    /**
     * _diaplayZoom is the global zoom used by Cairo.
     * It is set automatically depending on screen width
     * so that the borders of high-DPI displays are easily legible
     */
    double _displayZoom;

    /** A placeholder image to draw the frame onto*/
    UnTech::Image _frameImageBuffer;

    /** A pre-scaled copy of the frame image. */
    Glib::RefPtr<Gdk::Pixbuf> _framePixbuf;

    // The user supplied X/Y offset
    // on zoom/resize the _xOffset/_yOffset variables are changed.
    int _centerX, _centerY;

    std::shared_ptr<MS::Frame> _selectedFrame;
    Selection& _selection;
    Action _action;
};
}
}
}

#endif
