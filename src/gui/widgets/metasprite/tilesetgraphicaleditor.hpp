#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_TILESETGRAPHICALEDITOR_HPP_
#define _UNTECH_GUI_WIDGETS_METASPRITE_TILESETGRAPHICALEDITOR_HPP_

#include "tilesetgraphicaleditor.h"
#include "signals.h"
#include "gui/widgets/defaults.h"
#include "gui/widgets/common/cr_rgba.h"
#include "gui/undo/actionhelper.h"
#include "models/snes/tile.hpp"

#include <cmath>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

// Cannot use simple undo action for FrameObject::setSize
// Changing the size can change the location.
inline void frameObject_setTileIdAndSize(MS::FrameObject* item,
                                         const unsigned newTileId,
                                         const MS::FrameObject::ObjectSize& newSize)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(MS::FrameObject* item,
               const unsigned oldTileId,
               const MS::FrameObject::ObjectSize oldSize,
               const unsigned newTileId,
               const MS::FrameObject::ObjectSize newSize)
            : _item(item)
            , _oldTileId(oldTileId)
            , _oldSize(oldSize)
            , _newTileId(newTileId)
            , _newSize(newSize)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _item->setTileId(_oldTileId);
            _item->setSize(_oldSize);
            Signals::frameObjectChanged.emit(_item);
        }

        virtual void redo() override
        {
            _item->setTileId(_newTileId);
            _item->setSize(_newSize);
            Signals::frameObjectChanged.emit(_item);
        }

        virtual const Glib::ustring& message() const override
        {
            const static Glib::ustring message = _("Changed Object Tile");
            return message;
        }

    private:
        MS::FrameObject* _item;

        unsigned _oldTileId;
        MS::FrameObject::ObjectSize _oldSize;

        unsigned _newTileId;
        MS::FrameObject::ObjectSize _newSize;
    };

    if (item) {
        MS::FrameObject::ObjectSize oldSize = item->size();
        unsigned oldTileId = item->tileId();

        if (oldTileId != newTileId || oldSize != newSize) {
            item->setTileId(newTileId);
            item->setSize(newSize);
            Signals::frameObjectChanged.emit(item);

            Undo::UndoStack* undoStack = item->document().undoStack();
            if (undoStack) {
                undoStack->add_undo(std::make_unique<Action>(
                    item, oldTileId, oldSize, newTileId, newSize));
            }
        }
    }
}

template <class TilesetT>
inline void tileset_setPixel(MS::FrameSet* frameset,
                             TilesetT* tileset,
                             const unsigned tileId,
                             unsigned x, unsigned y,
                             unsigned value)
{
    class Action : public ::UnTech::Undo::MergeAction {
    public:
        Action() = delete;
        Action(MS::FrameSet* frameset,
               TilesetT* tileset,
               const unsigned tileId,
               const typename TilesetT::tile_t& oldTile,
               const typename TilesetT::tile_t& newTile)
            : _frameset(frameset)
            , _tileset(tileset)
            , _tileId(tileId)
            , _oldTile(oldTile)
            , _newTile(newTile)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _tileset->tile(_tileId) = _oldTile;
            Signals::frameSetTilesetChanged.emit(_frameset);
        }

        virtual void redo() override
        {
            _tileset->tile(_tileId) = _oldTile;
            Signals::frameSetTilesetChanged.emit(_frameset);
        }

        virtual bool mergeWith(::UnTech::Undo::MergeAction* o) override
        {
            Action* other = dynamic_cast<Action*>(o);

            if (other != nullptr) {
                if (this->_frameset == other->_frameset
                    && this->_tileset == other->_tileset
                    && this->_tileId == other->_tileId) {

                    this->_newTile = other->_newTile;
                    return true;
                }
            }

            return false;
        }

        virtual const Glib::ustring& message() const override
        {
            const static Glib::ustring message = _("Edit Tile");
            return message;
        }

    private:
        MS::FrameSet* _frameset;
        TilesetT* _tileset;
        unsigned _tileId;
        const typename TilesetT::tile_t _oldTile;
        typename TilesetT::tile_t _newTile;
    };

    if (tileset && tileId < tileset->size()) {
        typename TilesetT::tile_t oldTile = tileset->tile(tileId);

        tileset->tile(tileId).setPixel(x, y, value);

        typename TilesetT::tile_t newTile = tileset->tile(tileId);

        if (oldTile != newTile) {
            Signals::frameSetTilesetChanged.emit(frameset);

            Undo::UndoStack* undoStack = frameset->document().undoStack();
            if (undoStack) {
                undoStack->add_undoMerge(std::make_unique<Action>(
                    frameset, tileset, tileId, oldTile, newTile));
            }
        }
    }
}

template <>
inline UnTech::Snes::Tileset4bpp8px& TilesetGraphicalEditor<UnTech::Snes::Tileset4bpp8px>::tileset() const
{
    return _selection.frameSet()->smallTileset();
}

template <>
inline UnTech::Snes::Tileset4bpp16px& TilesetGraphicalEditor<UnTech::Snes::Tileset4bpp16px>::tileset() const
{
    return _selection.frameSet()->largeTileset();
}

template <class TilesetT>
TilesetGraphicalEditor<TilesetT>::TilesetGraphicalEditor(Selection& selection)
    : Gtk::Layout()
    , _selection(selection)
    , _zoomX(DEFAULT_ZOOM)
    , _zoomY(DEFAULT_ZOOM)
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

    Signals::frameObjectChanged.connect([this](const MS::FrameObject* obj) {
        if (obj && obj == _selection.frameObject()) {
            queue_draw();
        }
    });

    Signals::frameSetTilesetChanged.connect([this](const MS::FrameSet* fs) {
        if (fs && fs == _selection.frameSet()) {
            redrawTilesetPixbuf();
        }
    });

    Signals::frameSetTilesetCountChanged.connect([this](const MS::FrameSet* fs) {
        if (fs && fs == _selection.frameSet()) {
            redrawTilesetPixbuf();
        }
    });

    Signals::paletteChanged.connect([this](const MS::Palette* palette) {
        if (palette && palette == _selection.palette()) {
            redrawTilesetPixbuf();
        }
    });

    _selection.signal_frameSetChanged.connect(sigc::mem_fun(
        this, &TilesetGraphicalEditor::redrawTilesetPixbuf));

    _selection.signal_paletteChanged.connect(sigc::mem_fun(
        this, &TilesetGraphicalEditor::redrawTilesetPixbuf));

    _selection.signal_editTileColorChanged.connect([this](void) {
        _drawTileState = false;
        update_pointer_cursor();

        _selection.dontMergeNextUndoAction();
    });

    _selection.signal_frameObjectChanged.connect([this](void) {
        queue_draw();

        // scroll to object
        MS::FrameObject* obj = _selection.frameObject();
        if (obj && obj->sizePx() == TILE_SIZE) {
            double tileWidth = _zoomX * _displayZoom * TILE_SIZE;
            double xPos = tileWidth * obj->tileId();

            get_hadjustment()->clamp_page(xPos, xPos + tileWidth);
        }
    });
}

template <class TilesetT>
void TilesetGraphicalEditor<TilesetT>::redrawTilesetPixbuf()
{
    MS::Palette* palette = _selection.palette();

    if (_selection.frameSet() && palette && tileset().size() > 0) {
        const TilesetT& t = tileset();

        const unsigned width = t.size() * TILE_SIZE;
        const unsigned height = TILE_SIZE;

        // resize widget
        {
            double fWidth, fHeight;
            if (!std::isnan(_displayZoom)) {
                fWidth = width * _zoomX * _displayZoom;
                fHeight = height * _zoomY * _displayZoom;
            }
            else {
                fWidth = width * _zoomX;
                fHeight = height * _zoomY;
            }
            set_size(fWidth, fHeight);
            set_size_request(fWidth, fHeight);
        }

        if (_tilesetImageBuffer.size().width != width) {
            _tilesetImageBuffer = Image(width, height);
        }

        _tilesetImageBuffer.fill(palette->color(0).rgb());

        for (unsigned i = 0; i < t.size(); i++) {
            t.tile(i).draw(_tilesetImageBuffer, *palette,
                           i * TILE_SIZE, 0,
                           false, false);
        }

        auto pixbuf = Gdk::Pixbuf::create_from_data(reinterpret_cast<const guint8*>(_tilesetImageBuffer.data()),
                                                    Gdk::Colorspace::COLORSPACE_RGB, true, 8,
                                                    width, height,
                                                    width * 4);

        // Scaling is done by GTK not Cairo, as it results in sharp pixels
        _tilesetPixbuf = pixbuf->scale_simple(width * _zoomX,
                                              height * _zoomY,
                                              Gdk::InterpType::INTERP_NEAREST);
    }
    else {
        _tilesetPixbuf.reset();

        // resize widget
        {
            double fWidth, fHeight;
            if (!std::isnan(_displayZoom)) {
                fWidth = TILE_SIZE * _zoomX * _displayZoom;
                fHeight = TILE_SIZE * _zoomY * _displayZoom;
            }
            else {
                fWidth = TILE_SIZE * _zoomX;
                fHeight = TILE_SIZE * _zoomY;
            }
            set_size_request(fWidth, fHeight);
        }
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

    if (_selection.frameSet() == nullptr) {
        return true;
    }

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
        const TilesetT& ts = tileset();

        const double height = TILE_SIZE * _zoomY;

        for (unsigned i = 1; i < ts.size(); i++) {
            double x = TILE_SIZE * i * _zoomX;

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
    const MS::FrameObject* frameObject = _selection.frameObject();
    if (frameObject && frameObject->sizePx() == TILE_SIZE) {
        cr->set_line_width(1);

        const unsigned zX = frameObject->tileId() * TILE_SIZE * _zoomX;
        const double zWidth = TILE_SIZE * _zoomX;
        const double zHeight = TILE_SIZE * _zoomY;

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
        if (_selection.inEditTileMode()) {
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
        _selection.dontMergeNextUndoAction();
    }

    _drawTileState = false;

    if (_selection.frameSet() == nullptr) {
        return false;
    }

    MS::FrameObject* obj = _selection.frameObject();

    if (obj == nullptr) {
        return false;
    }

    if (event->button == 1) {
        const auto allocation = get_allocation();

        int x = std::lround((event->x - allocation.get_x()) / (_zoomX * _displayZoom));
        int y = std::lround((event->y - allocation.get_y()) / (_zoomY * _displayZoom));

        if (x >= 0 && y >= 0 && y < (int)TILE_SIZE) {
            unsigned tileId = (unsigned)x / TILE_SIZE;

            if (TILE_SIZE == 8) {
                if (tileId < _selection.frameSet()->smallTileset().size()) {
                    frameObject_setTileIdAndSize(obj, tileId, OS::SMALL);
                }
            }
            else {
                if (tileId < _selection.frameSet()->largeTileset().size()) {
                    frameObject_setTileIdAndSize(obj, tileId, OS::LARGE);
                }
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

    _selection.dontMergeNextUndoAction();

    return true;
}

template <class TilesetT>
void TilesetGraphicalEditor<TilesetT>::update_pointer_cursor()
{
    auto win = get_window();
    if (win) {
        if (_selection.inEditTileMode()) {
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
    if (_selection.frameSet() != nullptr && _selection.inEditTileMode()) {
        const auto allocation = get_allocation();

        const int x = std::lround((mouseX - allocation.get_x()) / (_zoomX * _displayZoom));
        const int y = std::lround((mouseY - allocation.get_y()) / (_zoomY * _displayZoom));

        if (x >= 0 && y >= 0 && y < (int)TILE_SIZE) {
            const unsigned tileId = (unsigned)x / TILE_SIZE;

            if (tileId < tileset().size()) {
                const unsigned tileX = (unsigned)x % TILE_SIZE;

                tileset_setPixel(_selection.frameSet(), &tileset(),
                                 tileId, tileX, y,
                                 (unsigned)_selection.editTileColor());
            }
        }
    }
}

inline double limit(double v, double min, double max)
{
    if (v < min) {
        return min;
    }
    else if (v > max) {
        return max;
    }
    else {
        return v;
    }
}

template <class TilesetT>
void TilesetGraphicalEditor<TilesetT>::setZoom(double x, double y)
{
    if (_zoomX != x || _zoomY != y) {
        _zoomX = limit(x, 1.0, 10.0);
        _zoomY = limit(y, 1.0, 10.0);

        redrawTilesetPixbuf();
    }
}
}
}
}

#endif
