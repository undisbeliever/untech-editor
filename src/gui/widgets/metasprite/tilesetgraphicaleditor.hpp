#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_TILESETGRAPHICALEDITOR_HPP_
#define _UNTECH_GUI_WIDGETS_METASPRITE_TILESETGRAPHICALEDITOR_HPP_

#include "tilesetgraphicaleditor.h"
#include "signals.h"
#include "gui/widgets/defaults.h"
#include "gui/widgets/common/cr_rgba.h"
#include "gui/undo/actionhelper.h"

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

            auto a = std::make_unique<Action>(item, oldTileId, oldSize, newTileId, newSize);

            auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(item->document()));
            undoDoc->undoStack().add_undo(std::move(a));
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
    , _zoomX(DEFAULT_ZOOM)
    , _zoomY(DEFAULT_ZOOM)
    , _displayZoom(NAN)
    , _tilesetImageBuffer()
    , _tilesetPixbuf()
    , _selection(selection)
{
    set_can_focus(true);

    add_events(Gdk::BUTTON_PRESS_MASK
               | Gdk::BUTTON_RELEASE_MASK);

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

    _selection.signal_frameObjectChanged.connect([this](void) {
        queue_draw();

        // scroll to object
        MS::FrameObject* obj = _selection.frameObject();
        if (obj && obj->sizePx() == TilesetT::TILE_SIZE) {
            double tileWidth = _zoomX * _displayZoom * TilesetT::TILE_SIZE;
            double xPos = tileWidth * obj->tileId();

            get_hadjustment()->clamp_page(xPos, xPos + tileWidth);
        }
    });
}

template <class TilesetT>
void TilesetGraphicalEditor<TilesetT>::redrawTilesetPixbuf()
{
    MS::Palette* palette = _selection.palette();

    if (_selection.frameSet() && palette) {
        const TilesetT& t = tileset();

        const unsigned width = t.size() * TilesetT::TILE_SIZE;
        const unsigned height = TilesetT::TILE_SIZE;

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

        // ::TODO handle hflip/vflip::
        for (unsigned i = 0; i < t.size(); i++) {
            t.drawTile(_tilesetImageBuffer, *palette,
                       i * TilesetT::TILE_SIZE, 0,
                       i, false, false);
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

        const double height = TilesetT::TILE_SIZE * _zoomY;

        for (unsigned i = 1; i < ts.size(); i++) {
            double x = TilesetT::TILE_SIZE * i * _zoomX;

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
    if (frameObject && frameObject->sizePx() == TilesetT::TILE_SIZE) {
        cr->set_line_width(1);

        const unsigned zX = frameObject->tileId() * TilesetT::TILE_SIZE * _zoomX;
        const double zWidth = TilesetT::TILE_SIZE * _zoomX;
        const double zHeight = TilesetT::TILE_SIZE * _zoomY;

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
bool TilesetGraphicalEditor<TilesetT>::on_button_release_event(GdkEventButton* event)
{
    grab_focus();

    if (_selection.frameSet() == nullptr) {
        return false;
    }

    if (event->button == 1) {
        const auto allocation = get_allocation();

        int x = std::lround((event->x - allocation.get_x()) / (_zoomX * _displayZoom));
        int y = std::lround((event->y - allocation.get_y()) / (_zoomY * _displayZoom));

        if (x >= 0 && y >= 0) {
            MS::FrameObject* obj = _selection.frameObject();

            if (obj) {
                typedef MS::FrameObject::ObjectSize OS;
                unsigned tileId = (unsigned)x / TilesetT::TILE_SIZE;

                if (TilesetT::TILE_SIZE == 8) {
                    frameObject_setTileIdAndSize(obj, tileId, OS::SMALL);
                }
                else {
                    frameObject_setTileIdAndSize(obj, tileId, OS::LARGE);
                }
            }
        }
    }
    return false;
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
