#pragma once

#include "gui/controllers/metasprite.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class FrameGraphicalEditor : public Gtk::DrawingArea {
public:
    FrameGraphicalEditor(MS::MetaSpriteController& controller);

    ~FrameGraphicalEditor() = default;

    void setFrame(const MS::Frame* frame);

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

    void on_nonPixmapDataChanged();
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
    MS::MetaSpriteController& _controller;
    const MS::Frame* _selectedFrame;

    int _xOffset, _yOffset;

    Pango::FontDescription _frameNameFont;

    /** A placeholder image to draw the frame onto*/
    UnTech::Image _frameImageBuffer;

    /** A pre-scaled copy of the frame image. */
    Glib::RefPtr<Gdk::Pixbuf> _framePixbuf;

    // The user supplied X/Y offset
    // on zoom/resize the _xOffset/_yOffset variables are changed.
    int _centerX, _centerY;

    Action _action;
};
}
}
}
