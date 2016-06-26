#pragma once

#include "tilesetgraphicaleditor.h"
#include "gui/widgets/common/cr_rgba.h"
#include "gui/widgets/defaults.h"
#include "models/snes/tile.hpp"

#include <cmath>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

template <class TilesetT>
TilesetGraphicalEditor<TilesetT>::TilesetGraphicalEditor(MS::MetaSpriteController& controller)
    : Gtk::Layout()
    , _controller(controller)
    , _displayZoom(NAN)
    , _tilesetImageBuffer()
    , _tilesetPixbuf()
    , _drawTileState(false)
{
    set_can_focus(true);

    add_events(Gdk::BUTTON_PRESS_MASK
               | Gdk::BUTTON_RELEASE_MASK
               | Gdk::ENTER_NOTIFY_MASK
               | Gdk::LEAVE_NOTIFY_MASK
               | Gdk::BUTTON1_MOTION_MASK);

    // SLOTS
    // =====
    auto& frameSetController = _controller.frameSetController();
    auto& paletteController = _controller.paletteController();
    auto& frameObjectController = _controller.frameObjectController();

    /* Controller Signals */
    _controller.settings().signal_zoomChanged().connect(sigc::mem_fun(
        *this, &TilesetGraphicalEditor::redrawTilesetPixbuf));

    frameSetController.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &TilesetGraphicalEditor::redrawTilesetPixbuf));

    frameSetController.template signal_tilesetChanged<TilesetT>().connect(
        [this](const MS::FrameSet* fs) {
            if (fs && fs == _controller.frameSetController().selected()) {
                redrawTilesetPixbuf();
            }
        });

    frameSetController.signal_tileCountChanged().connect([this](const MS::FrameSet* fs) {
        if (fs && fs == _controller.frameSetController().selected()) {
            redrawTilesetPixbuf();
        }
    });

    paletteController.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &TilesetGraphicalEditor::redrawTilesetPixbuf));

    paletteController.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &TilesetGraphicalEditor::redrawTilesetPixbuf));

    paletteController.signal_selectedColorChanged().connect([this](void) {
        _drawTileState = false;
        update_pointer_cursor();

        _controller.dontMergeNextAction();
    });

    frameObjectController.signal_selectedChanged().connect([this](void) {
        queue_draw();

        // scroll to object
        const MS::FrameObject* obj = _controller.frameObjectController().selected();
        if (obj && obj->sizePx() == TILE_SIZE) {
            const double zoomX = _controller.settings().zoomX();

            double tileWidth = zoomX * _displayZoom * TILE_SIZE;
            double xPos = tileWidth * obj->tileId();

            get_hadjustment()->clamp_page(xPos, xPos + tileWidth);
        }
    });

    frameObjectController.signal_dataChanged().connect([this](const MS::FrameObject* obj) {
        if (obj && obj == _controller.frameObjectController().selected()) {
            queue_draw();
        }
    });
}

template <class TilesetT>
void TilesetGraphicalEditor<TilesetT>::redrawTilesetPixbuf()
{
    const MS::Palette* palette = _controller.paletteController().selected();
    const MS::FrameSet* frameSet = _controller.frameSetController().selected();

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    if (frameSet == nullptr || palette == nullptr) {
        return clearPixbuf();
    }

    const TilesetT& tileset = frameSet->getTileset<TilesetT>();

    if (tileset.size() == 0) {
        return clearPixbuf();
    }

    const unsigned width = tileset.size() * TILE_SIZE;
    const unsigned height = TILE_SIZE;

    // resize widget
    {
        double fWidth, fHeight;
        if (!std::isnan(_displayZoom)) {
            fWidth = width * zoomX * _displayZoom;
            fHeight = height * zoomY * _displayZoom;
        }
        else {
            fWidth = width * zoomX;
            fHeight = height * zoomY;
        }
        set_size(fWidth, fHeight);
        set_size_request(fWidth, fHeight);
    }

    if (_tilesetImageBuffer.size().width != width) {
        _tilesetImageBuffer = Image(width, height);
    }

    _tilesetImageBuffer.fill(palette->color(0).rgb());

    for (unsigned i = 0; i < tileset.size(); i++) {
        tileset.tile(i).draw(_tilesetImageBuffer, *palette,
                             i * TILE_SIZE, 0,
                             false, false);
    }

    auto pixbuf = Gdk::Pixbuf::create_from_data(
        reinterpret_cast<const guint8*>(_tilesetImageBuffer.data()),
        Gdk::Colorspace::COLORSPACE_RGB, true, 8,
        width, height,
        width * 4);

    // Scaling is done by GTK not Cairo, as it results in sharp pixels
    _tilesetPixbuf = pixbuf->scale_simple(width * zoomX,
                                          height * zoomY,
                                          Gdk::InterpType::INTERP_NEAREST);

    queue_draw();
}

template <class TilesetT>
void TilesetGraphicalEditor<TilesetT>::clearPixbuf()
{
    _tilesetPixbuf.reset();

    // resize widget
    {
        const double zoomX = _controller.settings().zoomX();
        const double zoomY = _controller.settings().zoomY();

        double fWidth, fHeight;
        if (!std::isnan(_displayZoom)) {
            fWidth = TILE_SIZE * zoomX * _displayZoom;
            fHeight = TILE_SIZE * zoomY * _displayZoom;
        }
        else {
            fWidth = TILE_SIZE * zoomX;
            fHeight = TILE_SIZE * zoomY;
        }
        set_size_request(fWidth, fHeight);
    }
    queue_draw();
}

template <class TilesetT>
bool TilesetGraphicalEditor<TilesetT>::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // ::TODO move::
    const cr_rgba selectionInnerColor = { 1.0, 1.0, 1.0, 1.0 };
    const cr_rgba selectionOuterColor = { 0.0, 0.0, 0.0, 1.0 };

    const double TILE_BORDER_WIDTH = 1.0;
    const double TILE_BORDER_DASH = 3.0;
    const cr_rgba tileBorderColor1 = { 0.0, 0.0, 0.0, 0.9 };
    const cr_rgba tileBorderColor2 = { 1.0, 1.0, 1.0, 0.9 };

    if (std::isnan(_displayZoom)) {
        auto screen = get_screen();

        if (screen) {
            _displayZoom = std::floor(screen->get_width() / 1500) + 1.0;
        }
        else {
            _displayZoom = 1.0;
        }

        redrawTilesetPixbuf();
    }

    const MS::FrameSet* frameSet = _controller.frameSetController().selected();

    if (frameSet == nullptr) {
        return true;
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    const TilesetT& tileset = frameSet->getTileset<TilesetT>();

    cr->save();

    cr->translate(-get_hadjustment()->get_value(), 0);
    cr->scale(_displayZoom, _displayZoom);

    if (_tilesetPixbuf) {
        cr->set_antialias(Cairo::ANTIALIAS_NONE);

        Gdk::Cairo::set_source_pixbuf(cr, _tilesetPixbuf, 0, 0);

        cr->paint();
    }

    if (_displayZoom > 1.0) {
        cr->set_antialias(Cairo::ANTIALIAS_DEFAULT);
    }
    else {
        cr->set_antialias(Cairo::ANTIALIAS_NONE);
    }

    // Draw Frame Border
    {
        cr->save();

        static const std::vector<double> tileBorderDash({ TILE_BORDER_DASH, TILE_BORDER_DASH });
        const double height = TILE_SIZE * zoomY;

        for (unsigned i = 1; i < tileset.size(); i++) {
            double x = TILE_SIZE * i * zoomX;

            cr->move_to(x, 0);
            cr->line_to(x, height);
        }

        cr->set_line_width(TILE_BORDER_WIDTH);

        tileBorderColor1.apply(cr);
        cr->set_dash(tileBorderDash, TILE_BORDER_DASH / 2);
        cr->stroke_preserve();

        tileBorderColor2.apply(cr);
        cr->set_dash(tileBorderDash, TILE_BORDER_DASH + TILE_BORDER_DASH / 2);
        cr->stroke();

        cr->restore();
    }

    // draw selected tile
    const MS::FrameObject* frameObject = _controller.frameObjectController().selected();
    if (frameObject && frameObject->sizePx() == TILE_SIZE) {
        cr->set_line_width(1);

        const unsigned zX = frameObject->tileId() * TILE_SIZE * zoomX;
        const double zWidth = TILE_SIZE * zoomX;
        const double zHeight = TILE_SIZE * zoomY;

        selectionInnerColor.apply(cr);
        cr->rectangle(zX + 1, 1, zWidth - 2, zHeight - 2);
        cr->stroke();

        selectionOuterColor.apply(cr);
        cr->rectangle(zX, 0, zWidth, zHeight);
        cr->stroke();
    }

    cr->restore();

    return true;
}

template <class TilesetT>
bool TilesetGraphicalEditor<TilesetT>::on_button_press_event(GdkEventButton* event)
{
    grab_focus();

    if (event->button == 1) {
        if (isColorSelected()) {
            _drawTileState = true;
            setTilePixelForMouse(event->x, event->y);
        }
    }
    return false;
}

template <class TilesetT>
bool TilesetGraphicalEditor<TilesetT>::on_motion_notify_event(GdkEventMotion* event)
{
    if (_drawTileState) {
        setTilePixelForMouse(event->x, event->y);
    }
    return false;
}

template <class TilesetT>
bool TilesetGraphicalEditor<TilesetT>::on_button_release_event(GdkEventButton* event)
{
    typedef MS::FrameObject::ObjectSize OS;

    grab_focus();

    if (_drawTileState && event->button == 1) {
        _controller.dontMergeNextAction();
    }

    _drawTileState = false;

    const MS::FrameSet* frameSet = _controller.frameSetController().selected();
    const MS::FrameObject* frameObject = _controller.frameObjectController().selected();

    if (frameSet == nullptr || frameObject == nullptr) {
        return false;
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    const TilesetT& tileset = frameSet->getTileset<TilesetT>();

    if (event->button == 1) {
        const auto allocation = get_allocation();

        int x = std::lround((event->x - allocation.get_x()) / (zoomX * _displayZoom));
        int y = std::lround((event->y - allocation.get_y()) / (zoomY * _displayZoom));

        if (x >= 0 && y >= 0 && y < (int)TILE_SIZE) {
            unsigned tileId = (unsigned)x / TILE_SIZE;

            if (tileId < tileset.size()) {
                OS size = TILE_SIZE == 8 ? OS::SMALL : OS::LARGE;

                _controller.frameObjectController().selected_setSizeAndTileId(size, tileId);
            }
        }
    }
    return false;
}

template <class TilesetT>
bool TilesetGraphicalEditor<TilesetT>::on_enter_notify_event(GdkEventCrossing*)
{
    update_pointer_cursor();
    return true;
}

template <class TilesetT>
bool TilesetGraphicalEditor<TilesetT>::on_leave_notify_event(GdkEventCrossing*)
{
    auto win = get_window();
    if (win) {
        win->set_cursor();
    }

    _drawTileState = false;

    _controller.dontMergeNextAction();

    return true;
}

template <class TilesetT>
void TilesetGraphicalEditor<TilesetT>::update_pointer_cursor()
{
    auto win = get_window();
    if (win) {
        if (isColorSelected()) {
            win->set_cursor(Gdk::Cursor::create(get_display(), "pencil"));
        }
        else {
            win->set_cursor();
        }
    }
}

template <class TilesetT>
void TilesetGraphicalEditor<TilesetT>::setTilePixelForMouse(double mouseX, double mouseY)
{
    const MS::FrameSet* frameSet = _controller.frameSetController().selected();
    int colorId = _controller.paletteController().selectedColorId();

    if (frameSet != nullptr && colorId >= 0) {
        const double zoomX = _controller.settings().zoomX();
        const double zoomY = _controller.settings().zoomY();

        const auto allocation = get_allocation();

        // No rounding, pen cursor is at the top-left of pixel
        const int x = (mouseX - allocation.get_x()) / (zoomX * _displayZoom);
        const int y = (mouseY - allocation.get_y()) / (zoomY * _displayZoom);

        if (x >= 0 && y >= 0 && y < (int)TILE_SIZE) {
            const TilesetT& tileset = frameSet->getTileset<TilesetT>();

            const unsigned tileId = (unsigned)x / TILE_SIZE;
            if (tileId < tileset.size()) {
                const unsigned tileX = (unsigned)x % TILE_SIZE;

                auto& frameSetController = _controller.frameSetController();
                frameSetController.template selected_tileset_setPixel<TilesetT>(
                    tileId, tileX, y, (unsigned)colorId);
            }
        }
    }
}

template <class TilesetT>
bool TilesetGraphicalEditor<TilesetT>::isColorSelected()
{
    return _controller.paletteController().selectedColorId() >= 0;
}
}
}
}
