#pragma once

#include "gui/controllers/sprite-importer.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class FrameSetGraphicalEditor : public Gtk::DrawingArea {
public:
    FrameSetGraphicalEditor(SI::SpriteImporterController& controller);

    ~FrameSetGraphicalEditor() = default;

    void setZoom(double x, double y);

protected:
    struct Action {
        enum State {
            NONE = 0,
            CLICK,
            SELECT_TRANSPARENT_COLOR,
            DRAG
        };

        State state = NONE;
        upoint pressLocation;
        upoint previousLocation;

        urect dragAabb;
        bool canDrag;
        bool resize;
        bool resizeLeft;
        bool resizeRight;
        bool resizeTop;
        bool resizeBottom;
    };

    void resizeWidget();
    void on_dataChanged();
    void loadAndScaleImage();

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

    bool on_button_press_event(GdkEventButton* event) override;
    bool on_motion_notify_event(GdkEventMotion* event) override;
    bool on_button_release_event(GdkEventButton* event) override;

    void handleRelease_Click(const upoint& mouse);
    void handleRelease_SelectTransparentColor(const upoint& mouse);
    void handleRelease_Drag();

    bool on_enter_notify_event(GdkEventCrossing* event) override;
    bool on_leave_notify_event(GdkEventCrossing* event) override;

    void update_pointer_cursor();

private:
    SI::SpriteImporterController& _controller;

    double _zoomX, _zoomY;

    /**
     * _diaplayZoom is the global zoom used by Cairo.
     * It is set automatically depending on screen width
     * so that the borders of high-DPI displays are easily legible
     */
    double _displayZoom;

    // A pre-scaled copy of the frameset image.
    Glib::RefPtr<Gdk::Pixbuf> _frameSetImage;

    Action _action;
};
}
}
}
