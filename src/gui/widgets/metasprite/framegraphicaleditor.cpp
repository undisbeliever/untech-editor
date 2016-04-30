#include "framegraphicaleditor.h"
#include "signals.h"
#include "gui/undo/actionhelper.h"
#include "../common/cr_rgba.h"

#include <cmath>

using namespace UnTech::Widgets::MetaSprite;

const unsigned FRAME_IMAGE_SIZE = 256 + 16;
const unsigned FRAME_IMAGE_OFFSET = -UnTech::int_ms8_t::MIN;

SIMPLE_UNDO_ACTION(frameObject_setLocation,
                   MS::FrameObject, UnTech::ms8point, location, setLocation,
                   Signals::frameObjectChanged,
                   "Move Frame Object")

SIMPLE_UNDO_ACTION(actionPoint_setLocation,
                   MS::ActionPoint, UnTech::ms8point, location, setLocation,
                   Signals::actionPointChanged,
                   "Move Action Point")

SIMPLE_UNDO_ACTION(entityHitbox_setAabb,
                   MS::EntityHitbox, UnTech::ms8rect, aabb, setAabb,
                   Signals::entityHitboxChanged,
                   "Move Entity Hitbox")

FrameGraphicalEditor::FrameGraphicalEditor(Selection& selection)
    : Gtk::DrawingArea()
    , _zoomX(DEFAULT_ZOOM)
    , _zoomY(DEFAULT_ZOOM)
    , _displayZoom(NAN)
    , _frameNameFont("Monospace Bold")
    , _frameImageBuffer(FRAME_IMAGE_SIZE, FRAME_IMAGE_SIZE)
    , _framePixbuf()
    , _centerX()
    , _centerY()
    , _selectedFrame()
    , _selection(selection)
{
    set_hexpand(true);
    set_vexpand(true);
    set_can_focus(true);

    add_events(Gdk::BUTTON_PRESS_MASK
               | Gdk::BUTTON_RELEASE_MASK
               | Gdk::ENTER_NOTIFY_MASK
               | Gdk::LEAVE_NOTIFY_MASK
               | Gdk::BUTTON1_MOTION_MASK);

    // SLOTS
    // =====
    Signals::actionPointListChanged.connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::queue_draw)));
    Signals::entityHitboxListChanged.connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::queue_draw)));

    _selection.signal_selectionChanged.connect([this](void) {
        // reset action
        _action.state = Action::NONE;
        queue_draw();
    });

    Signals::frameChanged.connect([this](const MS::Frame* frame) {
        if (frame == _selectedFrame) {
            queue_draw();
        }
    });
    Signals::actionPointChanged.connect([this](const MS::ActionPoint* ap) {
        if (ap && &ap->frame() == _selectedFrame) {
            queue_draw();
        }
    });
    Signals::entityHitboxChanged.connect([this](const MS::EntityHitbox* eh) {
        if (eh && &eh->frame() == _selectedFrame) {
            queue_draw();
        }
    });

    // Redraw image if necessary
    _selection.signal_paletteChanged.connect(sigc::mem_fun(
        this, &FrameGraphicalEditor::redrawFramePixbuf));

    Signals::frameObjectListChanged.connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::redrawFramePixbuf)));

    Signals::frameObjectChanged.connect([this](const MS::FrameObject* obj) {
        if (obj && &obj->frame() == _selectedFrame) {
            redrawFramePixbuf();
        }
    });

    Signals::frameSetTilesetChanged.connect([this](const MS::FrameSet* frameSet) {
        if (_selectedFrame && &_selectedFrame->frameSet() == frameSet) {
            redrawFramePixbuf();
        }
    });

    Signals::paletteChanged.connect([this](const MS::Palette* palette) {
        if (palette && palette == _selection.palette()) {
            redrawFramePixbuf();
        }
    });

    // Update offsets on resize
    signal_size_allocate().connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::update_offsets)));

    // BUGFIX: update_offsets were not called on first draw, fixed by calling on frameChange::
    _selection.signal_frameChanged.connect(sigc::mem_fun(
        this, &FrameGraphicalEditor::update_offsets));
}

void FrameGraphicalEditor::setFrame(MS::Frame* frame)
{
    if (_selectedFrame != frame) {
        _selectedFrame = frame;
        redrawFramePixbuf();
    }
}

void FrameGraphicalEditor::redrawFramePixbuf()
{
    if (_selectedFrame && _selection.palette()) {
        // ::SHOULDO see if it is possible to edit pixmap data in UnTech::image ::

        // ::TODO fill with palette BG color?::

        _frameImageBuffer.fill(0);
        _selectedFrame->draw(_frameImageBuffer, *_selection.palette(),
                             FRAME_IMAGE_OFFSET, FRAME_IMAGE_OFFSET);

        auto pixbuf = Gdk::Pixbuf::create_from_data(reinterpret_cast<const guint8*>(_frameImageBuffer.data()),
                                                    Gdk::Colorspace::COLORSPACE_RGB, true, 8,
                                                    FRAME_IMAGE_SIZE, FRAME_IMAGE_SIZE,
                                                    FRAME_IMAGE_SIZE * 4);

        // Scaling is done by GTK not Cairo, as it results in sharp pixels
        _framePixbuf = pixbuf->scale_simple(FRAME_IMAGE_SIZE * _zoomX,
                                            FRAME_IMAGE_SIZE * _zoomY,
                                            Gdk::InterpType::INTERP_NEAREST);
    }
    else {
        _framePixbuf.reset();
    }

    queue_draw();
}

bool FrameGraphicalEditor::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // ::TODO move::
    const double ITEM_WIDTH = 1.0;
    const double OBJECT_DASH = 2.0;
    const double ACTION_POINT_SIZE = 1.5;
    const cr_rgba frameTileHitboxColor = { 0.8, 0.0, 0.0, 0.7 };
    const cr_rgba frameObjectColor = { 0.3, 0.9, 0.3, 0.7 };
    const cr_rgba actionPointColor = { 0.7, 1.0, 0.5, 0.95 };
    const cr_rgba entityHitboxColor = { 0.2, 0.0, 0.8, 0.7 };
    const cr_rgba selectionInnerColor = { 1.0, 1.0, 1.0, 1.0 };
    const cr_rgba selectionOuterColor = { 0.0, 0.0, 0.0, 1.0 };
    const cr_rgba selectionDragColor = { 0.5, 0.5, 0.5, 0.5 };

    const double ORIGIN_WIDTH = 1.0;
    const double ORIGIN_DASH = 3.0;
    const cr_rgba originColor1 = { 0.0, 0.0, 0.0, 0.2 };
    const cr_rgba originColor2 = { 1.0, 1.0, 1.0, 0.2 };

    if (std::isnan(_displayZoom)) {
        auto screen = get_screen();

        if (screen) {
            _displayZoom = std::floor(screen->get_width() / 1500) + 1.0;
        }
        else {
            _displayZoom = 1.0;
        }
    }

    if (_selectedFrame == nullptr) {
        return true;
    }

    const auto allocation = get_allocation();

    auto draw_rectangle = [this, cr](unsigned x, unsigned y, unsigned width, unsigned height) {
        cr->rectangle((x + _xOffset) * _zoomX, (y + _yOffset) * _zoomY,
                      width * _zoomX, height * _zoomY);
    };

    cr->save();

    cr->scale(_displayZoom, _displayZoom);

    if (_framePixbuf) {
        cr->set_antialias(Cairo::ANTIALIAS_NONE);

        Gdk::Cairo::set_source_pixbuf(cr, _framePixbuf,
                                      (_xOffset - (int)FRAME_IMAGE_OFFSET) * _zoomX,
                                      (_yOffset - (int)FRAME_IMAGE_OFFSET) * _zoomY);

        cr->paint();
    }

    if (_displayZoom > 1.0) {
        cr->set_antialias(Cairo::ANTIALIAS_DEFAULT);
    }
    else {
        cr->set_antialias(Cairo::ANTIALIAS_NONE);
    }

    const MS::Frame* frame = _selectedFrame;

    cr->set_line_width(ITEM_WIDTH);

    if (frame->solid()) {
        const auto& hb = frame->tileHitbox();

        draw_rectangle(hb.x, hb.y,
                       hb.width, hb.height);
        frameTileHitboxColor.apply(cr);
        cr->stroke();
    }

    for (const MS::EntityHitbox& eh : frame->entityHitboxes()) {
        const auto& aabb = eh.aabb();

        draw_rectangle(aabb.x, aabb.y,
                       aabb.width, aabb.height);

        // ::SHOULDO different color lines depending on type::
        entityHitboxColor.apply(cr);
        cr->stroke();
    }

    // outline objects
    {
        static const std::vector<double> objectDash({ OBJECT_DASH, OBJECT_DASH });

        cr->save();

        for (const MS::FrameObject& obj : frame->objects()) {
            const auto oloc = obj.location();

            draw_rectangle(oloc.x, oloc.y,
                           obj.sizePx(), obj.sizePx());
        }

        frameObjectColor.apply(cr);
        cr->set_dash(objectDash, 0);
        cr->stroke();
        cr->restore();
    }

    for (const MS::ActionPoint& ap : frame->actionPoints()) {
        const auto aLoc = ap.location();

        double aWidth = ACTION_POINT_SIZE * _zoomX / 2;
        double aHeight = ACTION_POINT_SIZE * _zoomY / 2;
        double x = (_xOffset + aLoc.x + 0.5) * _zoomX;
        double y = (_yOffset + aLoc.y + 0.5) * _zoomY;

        cr->move_to(x, y - aHeight);
        cr->line_to(x, y + aHeight);
        cr->move_to(x - aWidth, y);
        cr->line_to(x + aWidth, y);

        // ::SHOULDO different color lines depending on type::
        actionPointColor.apply(cr);
        cr->stroke();
    }

    // Draw Origin
    {
        cr->save();

        static const std::vector<double> originDash({ ORIGIN_DASH, ORIGIN_DASH });

        const unsigned aWidth = allocation.get_width();
        const unsigned aHeight = allocation.get_height();

        double x = _xOffset * _zoomX;
        double y = _yOffset * _zoomY;

        cr->move_to(x, y);
        cr->line_to(0, y);

        cr->move_to(x, y);
        cr->line_to(aWidth, y);

        cr->move_to(x, y);
        cr->line_to(x, 0);

        cr->move_to(x, y);
        cr->line_to(x, aHeight);

        cr->set_line_width(ORIGIN_WIDTH);

        originColor1.apply(cr);
        cr->set_dash(originDash, ORIGIN_DASH / 2);
        cr->stroke_preserve();

        originColor2.apply(cr);
        cr->set_dash(originDash, ORIGIN_DASH + ORIGIN_DASH / 2);
        cr->stroke();

        cr->restore();
    }

    auto draw_selected_rectangle = [&](unsigned x, unsigned y,
                                       unsigned width, unsigned height) {
        cr->set_line_width(1);

        const double zX = (x + _xOffset) * _zoomX + 1;
        const double zY = (y + _yOffset) * _zoomY + 1;
        const double zWidth = width * _zoomX - 1;
        const double zHeight = height * _zoomY - 1;

        cr->rectangle(zX, zY, zWidth, zHeight);
        cr->stroke();

        selectionInnerColor.apply(cr);
        cr->rectangle(zX + 1, zY + 1, zWidth - 2, zHeight - 2);
        cr->stroke();

        selectionOuterColor.apply(cr);
        cr->rectangle(zX - 1, zY - 1, zWidth + 2, zHeight + 2);
        cr->stroke();
    };

    if (_selection.frame() == _selectedFrame) {
        switch (_selection.type()) {
        case Selection::Type::NONE:
            break;

        case Selection::Type::FRAME_OBJECT:
            if (_selection.frameObject()) {
                const auto oLoc = _selection.frameObject()->location();
                const unsigned oSize = _selection.frameObject()->sizePx();

                frameObjectColor.apply(cr);
                draw_selected_rectangle(oLoc.x, oLoc.y, oSize, oSize);
            }
            break;

        case Selection::Type::ENTITY_HITBOX:
            if (_selection.entityHitbox()) {
                const auto aabb = _selection.entityHitbox()->aabb();

                // ::SHOULDO different color lines depending on type::
                entityHitboxColor.apply(cr);
                draw_selected_rectangle(aabb.x, aabb.y, aabb.width, aabb.height);
            }
            break;

        case Selection::Type::ACTION_POINT:
            if (_selection.actionPoint()) {
                const auto aLoc = _selection.actionPoint()->location();

                cr->save();

                double aWidth = ACTION_POINT_SIZE * _zoomX / 2;
                double aHeight = ACTION_POINT_SIZE * _zoomY / 2;
                double x = (aLoc.x + _xOffset + 0.5) * _zoomX;
                double y = (aLoc.y + _yOffset + 0.5) * _zoomY;

                cr->move_to(x, y - aHeight - 1.0);
                cr->line_to(x, y + aHeight + 1.0);
                cr->move_to(x - aWidth - 1.0, y);
                cr->line_to(x + aWidth + 1.0, y);

                selectionOuterColor.apply(cr);
                cr->set_line_width(3.0);
                cr->stroke();

                cr->move_to(x, y - aHeight);
                cr->line_to(x, y + aHeight);
                cr->move_to(x - aWidth, y);
                cr->line_to(x + aWidth, y);

                selectionInnerColor.apply(cr);
                cr->set_line_width(1.0);
                cr->stroke();

                cr->restore();
            }
            break;
        }
    }

    if (_action.state == Action::DRAG) {
        const ms8rect aabb = _action.dragAabb;

        // ::SHOULDDO checkerboard pattern::

        draw_rectangle(aabb.x, aabb.y, aabb.width, aabb.height);
        selectionDragColor.apply(cr);
        cr->fill();
    }

    cr->restore();

    // Draw frame name
    {
        auto name = _selectedFrame->frameSet().frames().getName(_selectedFrame);

        if (name.second) {
            cr->save();

            // Use the default color of the current style
            Gdk::Cairo::set_source_rgba(cr,
                                        get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL));

            auto layout = create_pango_layout(name.first);
            layout->set_font_description(_frameNameFont);

            int textWidth, textHeight;
            layout->get_pixel_size(textWidth, textHeight);

            // Draw text in bottom left.
            cr->move_to(DEFAULT_ROW_SPACING * 2,
                        allocation.get_height() - textHeight - DEFAULT_ROW_SPACING);
            layout->show_in_cairo_context(cr);

            cr->restore();
        }
    }

    return true;
}

bool FrameGraphicalEditor::on_button_press_event(GdkEventButton* event)
{
    if (_selectedFrame == nullptr) {
        return false;
    }

    if (event->button == 1) {
        if (_action.state == Action::NONE) {
            auto allocation = get_allocation();

            int mouseX = std::lround((event->x - allocation.get_x()) / (_zoomX * _displayZoom)) - _xOffset;
            int mouseY = std::lround((event->y - allocation.get_y()) / (_zoomY * _displayZoom)) - _yOffset;

            _action.canDrag = false;

            _action.state = Action::CLICK;
            _action.pressLocation = ms8point(mouseX, mouseY);
            ms8rect aabb;

            switch (_selection.type()) {
            case Selection::Type::NONE:
                break;

            case Selection::Type::FRAME_OBJECT:
                if (_selection.frameObject()) {
                    const MS::FrameObject* fo = _selection.frameObject();
                    const auto foLoc = fo->location();

                    aabb = ms8rect(foLoc.x, foLoc.y, fo->sizePx(), fo->sizePx());
                    _action.canDrag = true;
                }
                break;

            case Selection::Type::ACTION_POINT:
                if (_selection.actionPoint()) {
                    const MS::ActionPoint* ap = _selection.actionPoint();
                    const auto apLoc = ap->location();

                    aabb = ms8rect(apLoc.x, apLoc.y, 1, 1);
                    _action.canDrag = true;
                }
                break;

            case Selection::Type::ENTITY_HITBOX:
                if (_selection.entityHitbox()) {
                    const MS::EntityHitbox* eh = _selection.entityHitbox();

                    aabb = eh->aabb();
                    _action.canDrag = true;
                }
                break;
            }

            if (_action.canDrag) {
                _action.dragAabb = aabb;

                const ms8point fm(mouseX, mouseY);

                if (_selection.type() == Selection::Type::ENTITY_HITBOX) {

                    _action.resizeLeft = fm.x == aabb.left();
                    _action.resizeRight = fm.x == aabb.right();
                    _action.resizeTop = fm.y == aabb.top();
                    _action.resizeBottom = fm.y == aabb.bottom();

                    _action.resize = _action.resizeLeft | _action.resizeRight
                                     | _action.resizeTop | _action.resizeBottom;
                }
                else {
                    _action.resize = false;
                }

                // make sure click is inside the item
                _action.canDrag = aabb.contains(fm) || _action.resize;
            }
        }
    }
    return false;
}

bool FrameGraphicalEditor::on_motion_notify_event(GdkEventMotion* event)
{
    if (_selectedFrame == nullptr) {
        _action.state = Action::NONE;
        return false;
    }

    if (_action.state != Action::NONE) {
        auto allocation = get_allocation();

        int mouseX = std::lround((event->x - allocation.get_x()) / (_zoomX * _displayZoom)) - _xOffset;
        int mouseY = std::lround((event->y - allocation.get_y()) / (_zoomY * _displayZoom)) - _yOffset;

        ms8point mouse(mouseX, mouseY);

        if (_action.state == Action::CLICK && _action.canDrag) {
            if (_action.pressLocation != mouse) {
                _action.state = Action::DRAG;
                _action.previousLocation = _action.pressLocation;

                update_pointer_cursor();
            }
        }

        if (_action.state == Action::DRAG) {
            // move dragAbbb to new location.
            if (_action.previousLocation != mouse) {
                ms8rect aabb = _action.dragAabb;

                if (!_action.resize) {
                    // move
                    aabb.x += mouse.x - _action.previousLocation.x;
                    aabb.y += mouse.y - _action.previousLocation.y;
                }
                else {
                    // resize
                    int rx = mouse.x;
                    if (_action.resizeLeft) {
                        if (rx >= aabb.right()) {
                            rx = aabb.right() - 1;
                        }
                        aabb.width = aabb.right() - rx;
                        aabb.x = rx;
                    }
                    else if (_action.resizeRight) {
                        if (rx <= aabb.left()) {
                            rx = aabb.left() + 1;
                        }
                        aabb.width = rx - aabb.x;
                    }

                    int ry = mouse.y;
                    if (_action.resizeTop) {
                        if (ry >= aabb.bottom()) {
                            ry = aabb.bottom() - 1;
                        }
                        aabb.height = aabb.bottom() - ry;
                        aabb.y = ry;
                    }
                    else if (_action.resizeBottom) {
                        if (ry <= aabb.top()) {
                            ry = aabb.top() + 1;
                        }
                        aabb.height = ry - aabb.y;
                    }
                }

                if (_action.dragAabb != aabb) {
                    _action.dragAabb = aabb;
                    queue_draw();
                }

                _action.previousLocation = mouse;
            }
        }
    }
    return false;
}

bool FrameGraphicalEditor::on_button_release_event(GdkEventButton* event)
{
    grab_focus();

    if (_selectedFrame == nullptr) {
        return false;
    }

    if (event->button == 1) {
        auto allocation = get_allocation();

        int x = std::lround((event->x - allocation.get_x()) / (_zoomX * _displayZoom)) - _xOffset;
        int y = std::lround((event->y - allocation.get_y()) / (_zoomY * _displayZoom)) - _yOffset;

        ms8point mouse(x, y);

        switch (_action.state) {
        case Action::CLICK:
            handleRelease_Click(mouse);
            break;

        case Action::DRAG:
            handleRelease_Drag();
            break;

        case Action::NONE:
            break;
        }
    }
    return false;
}

void FrameGraphicalEditor::handleRelease_Click(const ms8point& mouse)
{
    _action.state = Action::NONE;

    // only select item if mouse didn't move.
    if (mouse != _action.pressLocation) {
        return;
    }

    /*
     * Select a given item.
     *
     * This code cycles through the given selections.
     * On click, the next item is selected. If the last item
     * was the previously selected one then the first match
     * is selected.
     */
    struct SelHandler {
        Selection::Type type = Selection::Type::NONE;
        MS::FrameObject* frameObject = nullptr;
        MS::ActionPoint* actionPoint = nullptr;
        MS::EntityHitbox* entityHitbox = nullptr;
    };
    SelHandler current;
    SelHandler firstMatch;

    const MS::Frame* sFrame = _selectedFrame;

    for (MS::FrameObject& obj : sFrame->objects()) {
        const auto loc = obj.location();

        if (mouse.x >= loc.x && mouse.x < (loc.x + (int)obj.sizePx())
            && mouse.y >= loc.y && mouse.y < (loc.y + (int)obj.sizePx())) {
            if (current.type == Selection::Type::NONE) {
                current.frameObject = &obj;
                current.type = Selection::Type::FRAME_OBJECT;

                if (firstMatch.type == Selection::Type::NONE) {
                    firstMatch.frameObject = &obj;
                    firstMatch.type = Selection::Type::FRAME_OBJECT;
                }
            }
            if (_selection.frameObject() == &obj
                && _selection.type() == Selection::Type::FRAME_OBJECT) {

                current.type = Selection::Type::NONE;
            }
        }
    }

    for (MS::ActionPoint& ap : sFrame->actionPoints()) {
        const auto loc = ap.location();

        if (mouse == loc) {
            if (current.type == Selection::Type::NONE) {
                current.actionPoint = &ap;
                current.type = Selection::Type::ACTION_POINT;

                if (firstMatch.type == Selection::Type::NONE) {
                    firstMatch.actionPoint = &ap;
                    firstMatch.type = Selection::Type::ACTION_POINT;
                }
            }
            if (_selection.actionPoint() == &ap
                && _selection.type() == Selection::Type::ACTION_POINT) {

                current.type = Selection::Type::NONE;
            }
        }
    }

    for (MS::EntityHitbox& eh : sFrame->entityHitboxes()) {
        const auto aabb = eh.aabb();

        if (aabb.contains(mouse)) {
            if (current.type == Selection::Type::NONE) {
                current.entityHitbox = &eh;
                current.type = Selection::Type::ENTITY_HITBOX;

                if (firstMatch.type == Selection::Type::NONE) {
                    firstMatch.entityHitbox = &eh;
                    firstMatch.type = Selection::Type::ENTITY_HITBOX;
                }
            }
            if (_selection.entityHitbox() == &eh
                && _selection.type() == Selection::Type::ENTITY_HITBOX) {
                current.type = Selection::Type::NONE;
            }
        }
    }

    if (current.type == Selection::Type::NONE) {
        // handle wrap around.
        current = firstMatch;
    }

    switch (current.type) {
    case Selection::Type::NONE:
        _selection.unselectAll();
        break;

    case Selection::Type::FRAME_OBJECT:
        _selection.setFrameObject(current.frameObject);
        break;

    case Selection::Type::ACTION_POINT:
        _selection.setActionPoint(current.actionPoint);
        break;

    case Selection::Type::ENTITY_HITBOX:
        _selection.setEntityHitbox(current.entityHitbox);
        break;
    }
}

void FrameGraphicalEditor::handleRelease_Drag()
{
    _action.state = Action::NONE;
    update_pointer_cursor();

    const auto aabb = _action.dragAabb;

    switch (_selection.type()) {
    case Selection::Type::NONE:
        break;

    case Selection::Type::FRAME_OBJECT:
        if (_selection.frameObject()) {
            frameObject_setLocation(_selection.frameObject(),
                                    ms8point(aabb.x, aabb.y));
        }
        break;

    case Selection::Type::ACTION_POINT:
        if (_selection.actionPoint()) {
            actionPoint_setLocation(_selection.actionPoint(),
                                    ms8point(aabb.x, aabb.y));
        }
        break;

    case Selection::Type::ENTITY_HITBOX:
        if (_selection.entityHitbox()) {
            entityHitbox_setAabb(_selection.entityHitbox(), aabb);
        }
        break;
    }

    queue_draw();
}

bool FrameGraphicalEditor::on_enter_notify_event(GdkEventCrossing*)
{
    update_pointer_cursor();
    return true;
}

void FrameGraphicalEditor::update_pointer_cursor()
{
    auto win = get_window();
    if (win) {
        switch (_action.state) {
        case Action::NONE:
        case Action::CLICK:
            win->set_cursor();
            break;

        case Action::DRAG:
            if (!_action.resize) {
                win->set_cursor(Gdk::Cursor::create(get_display(), "move"));
            }
            else {
                bool horizontal = _action.resizeLeft | _action.resizeRight;
                bool vertical = _action.resizeTop | _action.resizeBottom;

                if (horizontal && !vertical) {
                    win->set_cursor(Gdk::Cursor::create(get_display(), "ew-resize"));
                }
                else if (vertical && !horizontal) {
                    win->set_cursor(Gdk::Cursor::create(get_display(), "ns-resize"));
                }
                else if ((_action.resizeLeft && _action.resizeTop)
                         || (_action.resizeRight && _action.resizeBottom)) {
                    win->set_cursor(Gdk::Cursor::create(get_display(), "nwse-resize"));
                }
                else if ((_action.resizeRight && _action.resizeTop)
                         || (_action.resizeLeft && _action.resizeBottom)) {
                    win->set_cursor(Gdk::Cursor::create(get_display(), "nesw-resize"));
                }
            }
            break;
        }
    }
}

bool FrameGraphicalEditor::on_leave_notify_event(GdkEventCrossing*)
{
    auto win = get_window();
    if (win) {
        win->set_cursor();
    }

    return true;
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

void FrameGraphicalEditor::setZoom(double x, double y)
{
    if (_zoomX != x || _zoomY != y) {
        _zoomX = limit(x, 1.0, 10.0);
        _zoomY = limit(y, 1.0, 10.0);

        update_offsets();
        redrawFramePixbuf();
    }
}

void FrameGraphicalEditor::setCenter(int x, int y)
{
    if (_centerX != x || _centerY != y) {
        _centerX = x;
        _centerY = y;

        update_offsets();
    }
}

void FrameGraphicalEditor::update_offsets()
{
    const auto allocation = get_allocation();

    _xOffset = allocation.get_width() / _zoomX / _displayZoom / 2 + _centerX;
    _yOffset = allocation.get_height() / _zoomY / _displayZoom / 2 + _centerY;

    queue_draw();
}
