#include "framegraphicaleditor.h"
#include "../common/cr_rgba.h"

#include <cmath>

using namespace UnTech::Widgets::MetaSprite;

typedef MS::MetaSpriteController::SelectedTypeController::Type SelectedType;

const unsigned FRAME_IMAGE_SIZE = 256 + 16;
const unsigned FRAME_IMAGE_OFFSET = -UnTech::int_ms8_t::MIN;

FrameGraphicalEditor::FrameGraphicalEditor(MS::MetaSpriteController& controller)
    : Gtk::DrawingArea()
    , _controller(controller)
    , _selectedFrame(nullptr)
    , _frameNameFont("Monospace Bold")
    , _frameImageBuffer(FRAME_IMAGE_SIZE, FRAME_IMAGE_SIZE)
    , _framePixbuf()
    , _centerX()
    , _centerY()
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

    /* Controller Signals */
    _controller.settings().signal_zoomChanged().connect([this](void) {
        update_offsets();
        redrawFramePixbuf();
    });

    _controller.frameController().signal_listDataChanged().connect(
        [this](const MS::Frame::list_t* list) {
            if (list == nullptr || !list->contains(_selectedFrame)) {
                setFrame(nullptr);
            }
        });
    _controller.frameController().signal_listChanged().connect(
        [this](void) {
            auto list = _controller.frameController().list();
            if (list == nullptr || !list->contains(_selectedFrame)) {
                setFrame(nullptr);
            }
        });

    _controller.frameController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::on_nonPixmapDataChanged)));

    _controller.frameObjectController().signal_selectedChanged().connect(sigc::mem_fun(
        this, &FrameGraphicalEditor::on_nonPixmapDataChanged));

    _controller.entityHitboxController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::on_nonPixmapDataChanged)));
    _controller.entityHitboxController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::on_nonPixmapDataChanged)));
    _controller.entityHitboxController().signal_selectedChanged().connect(sigc::mem_fun(
        this, &FrameGraphicalEditor::on_nonPixmapDataChanged));

    _controller.actionPointController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::on_nonPixmapDataChanged)));
    _controller.actionPointController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::on_nonPixmapDataChanged)));
    _controller.actionPointController().signal_selectedChanged().connect(sigc::mem_fun(
        this, &FrameGraphicalEditor::on_nonPixmapDataChanged));

    _controller.selectedTypeController().signal_typeChanged().connect(sigc::mem_fun(
        this, &FrameGraphicalEditor::on_nonPixmapDataChanged));

    // Redraw buffer when obj/palette/tiles changed
    _controller.frameObjectController().signal_dataChanged().connect(
        [this](const MS::FrameObject* obj) {
            if (obj && _selectedFrame == &obj->frame()) {
                redrawFramePixbuf();
            }
        });

    _controller.frameObjectController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::redrawFramePixbuf)));

    _controller.paletteController().signal_selectedChanged().connect(sigc::mem_fun(
        this, &FrameGraphicalEditor::redrawFramePixbuf));

    _controller.paletteController().signal_selectedDataChanged().connect(sigc::mem_fun(
        this, &FrameGraphicalEditor::redrawFramePixbuf));

    _controller.frameSetController().signal_smallTilesetChanged().connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::redrawFramePixbuf)));

    _controller.frameSetController().signal_largeTilesetChanged().connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::redrawFramePixbuf)));

    // Update offsets on resize
    signal_size_allocate().connect(sigc::hide(sigc::mem_fun(
        this, &FrameGraphicalEditor::update_offsets)));

    // BUGFIX: update_offsets were not called on first draw,
    //         fixed by calling on frame change::
    _controller.frameController().signal_selectedChanged().connect(sigc::mem_fun(
        this, &FrameGraphicalEditor::update_offsets));
}

void FrameGraphicalEditor::setFrame(const MS::Frame* frame)
{
    if (_selectedFrame != frame) {
        _selectedFrame = frame;
        redrawFramePixbuf();
    }
}

void FrameGraphicalEditor::on_nonPixmapDataChanged()
{
    if (_action.state != Action::NONE) {
        _action.state = Action::NONE;
        update_pointer_cursor();
    }
    queue_draw();
}

void FrameGraphicalEditor::redrawFramePixbuf()
{
    const MS::Palette* palette = _controller.paletteController().selected();

    if (_selectedFrame && palette) {
        // ::SHOULDO see if it is possible to edit pixmap data in UnTech::image ::

        // ::TODO fill with palette BG color?::

        const double zoomX = _controller.settings().zoomX();
        const double zoomY = _controller.settings().zoomY();

        _frameImageBuffer.fill(UnTech::rgba::fromRgba(0));
        _selectedFrame->draw(_frameImageBuffer, *palette,
                             FRAME_IMAGE_OFFSET, FRAME_IMAGE_OFFSET);

        auto pixbuf = Gdk::Pixbuf::create_from_data(reinterpret_cast<const guint8*>(_frameImageBuffer.data()),
                                                    Gdk::Colorspace::COLORSPACE_RGB, true, 8,
                                                    FRAME_IMAGE_SIZE, FRAME_IMAGE_SIZE,
                                                    FRAME_IMAGE_SIZE * 4);

        // Scaling is done by GTK not Cairo, as it results in sharp pixels
        _framePixbuf = pixbuf->scale_simple(FRAME_IMAGE_SIZE * zoomX,
                                            FRAME_IMAGE_SIZE * zoomY,
                                            Gdk::InterpType::INTERP_NEAREST);
    }
    else {
        _framePixbuf.reset();
    }

    on_nonPixmapDataChanged();
}

bool FrameGraphicalEditor::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    const double ORIGIN_WIDTH = 1.0;
    const double ORIGIN_DASH = 3.0;

    // ::TODO move::
    const cr_rgba frameTileHitboxColor = { 0.8, 0.0, 0.0, 0.7 };
    const cr_rgba frameObjectColor = { 0.3, 0.9, 0.3, 0.7 };
    const cr_rgba actionPointColor = { 0.7, 1.0, 0.5, 0.95 };
    const cr_rgba entityHitboxColor = { 0.2, 0.0, 0.8, 0.7 };
    const cr_rgba selectionInnerColor = { 1.0, 1.0, 1.0, 1.0 };
    const cr_rgba selectionOuterColor = { 0.0, 0.0, 0.0, 1.0 };
    const cr_rgba selectionDragColor = { 0.5, 0.5, 0.5, 0.5 };

    const double lineWidth = _controller.settings().lineWidth();
    const double actionPointSize = 1.5 * lineWidth;

    const cr_rgba originColor1 = { 0.0, 0.0, 0.0, 0.2 };
    const cr_rgba originColor2 = { 1.0, 1.0, 1.0, 0.2 };

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    if (_selectedFrame == nullptr) {
        return true;
    }

    const auto allocation = get_allocation();

    auto draw_rectangle = [&](unsigned x, unsigned y, unsigned width, unsigned height) {
        cr->rectangle((x + _xOffset) * zoomX, (y + _yOffset) * zoomY,
                      width * zoomX, height * zoomY);
    };

    cr->save();

    cr->set_antialias(Cairo::ANTIALIAS_NONE);

    if (_framePixbuf) {
        Gdk::Cairo::set_source_pixbuf(cr, _framePixbuf,
                                      (_xOffset - (int)FRAME_IMAGE_OFFSET) * zoomX,
                                      (_yOffset - (int)FRAME_IMAGE_OFFSET) * zoomY);

        cr->paint();
    }

    if (lineWidth > 1) {
        cr->set_antialias(Cairo::ANTIALIAS_DEFAULT);
    }

    const MS::Frame* frame = _selectedFrame;

    cr->set_line_width(_controller.settings().lineWidth());

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
        cr->save();

        const double objectDashLength = lineWidth * 2;
        const std::vector<double> objectDash({ objectDashLength, objectDashLength });

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

        double aWidth = actionPointSize * zoomX / 2;
        double aHeight = actionPointSize * zoomY / 2;
        double x = (_xOffset + aLoc.x + 0.5) * zoomX;
        double y = (_yOffset + aLoc.y + 0.5) * zoomY;

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

        double x = _xOffset * zoomX;
        double y = _yOffset * zoomY;

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
        cr->set_dash(originDash, ORIGIN_DASH / 2 * 3);
        cr->stroke();

        cr->restore();
    }

    auto draw_selected_rectangle = [&](unsigned x, unsigned y,
                                       unsigned width, unsigned height) {
        cr->set_line_width(1);

        const double zX = (x + _xOffset) * zoomX + 1;
        const double zY = (y + _yOffset) * zoomY + 1;
        const double zWidth = width * zoomX - 1;
        const double zHeight = height * zoomY - 1;

        cr->rectangle(zX, zY, zWidth, zHeight);
        cr->stroke();

        selectionInnerColor.apply(cr);
        cr->rectangle(zX + 1, zY + 1, zWidth - 2, zHeight - 2);
        cr->stroke();

        selectionOuterColor.apply(cr);
        cr->rectangle(zX - 1, zY - 1, zWidth + 2, zHeight + 2);
        cr->stroke();
    };

    if (_selectedFrame == _controller.frameController().selected()) {
        switch (_controller.selectedTypeController().type()) {
        case SelectedType::NONE:
            break;

        case SelectedType::FRAME_OBJECT:
            if (const auto* fo = _controller.frameObjectController().selected()) {
                const auto oLoc = fo->location();
                const unsigned oSize = fo->sizePx();

                frameObjectColor.apply(cr);
                draw_selected_rectangle(oLoc.x, oLoc.y, oSize, oSize);
            }
            break;

        case SelectedType::ENTITY_HITBOX:
            if (const auto* eh = _controller.entityHitboxController().selected()) {

                const auto aabb = eh->aabb();

                // ::SHOULDO different color lines depending on type::
                entityHitboxColor.apply(cr);
                draw_selected_rectangle(aabb.x, aabb.y, aabb.width, aabb.height);
            }
            break;

        case SelectedType::ACTION_POINT:
            if (const auto* ap = _controller.actionPointController().selected()) {
                const auto aLoc = ap->location();

                cr->save();

                double aWidth = actionPointSize * zoomX / 2;
                double aHeight = actionPointSize * zoomY / 2;
                double x = (aLoc.x + _xOffset + 0.5) * zoomX;
                double y = (aLoc.y + _yOffset + 0.5) * zoomY;

                cr->move_to(x, y - aHeight - lineWidth);
                cr->line_to(x, y + aHeight + lineWidth);
                cr->move_to(x - aWidth - lineWidth, y);
                cr->line_to(x + aWidth + lineWidth, y);

                selectionOuterColor.apply(cr);
                cr->set_line_width(lineWidth * 3);
                cr->stroke();

                cr->move_to(x, y - aHeight);
                cr->line_to(x, y + aHeight);
                cr->move_to(x - aWidth, y);
                cr->line_to(x + aWidth, y);

                selectionInnerColor.apply(cr);
                cr->set_line_width(lineWidth);
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

        if (name) {
            cr->save();

            // Use the default color of the current style
            Gdk::Cairo::set_source_rgba(cr,
                                        get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL));

            auto layout = create_pango_layout(name.value());
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

            const double zoomX = _controller.settings().zoomX();
            const double zoomY = _controller.settings().zoomY();

            int mouseX = std::lround((event->x - allocation.get_x()) / zoomX) - _xOffset;
            int mouseY = std::lround((event->y - allocation.get_y()) / zoomY) - _yOffset;

            _action.canDrag = false;

            _action.state = Action::CLICK;
            _action.pressLocation = ms8point(mouseX, mouseY);
            ms8rect aabb;

            const auto selectedType = _controller.selectedTypeController().type();

            switch (selectedType) {
            case SelectedType::NONE:
                break;

            case SelectedType::FRAME_OBJECT:
                if (const auto* fo = _controller.frameObjectController().selected()) {
                    const auto foLoc = fo->location();

                    aabb = ms8rect(foLoc.x, foLoc.y, fo->sizePx(), fo->sizePx());
                    _action.canDrag = true;
                }
                break;

            case SelectedType::ACTION_POINT:
                if (const auto* ap = _controller.actionPointController().selected()) {
                    const auto apLoc = ap->location();

                    aabb = ms8rect(apLoc.x, apLoc.y, 1, 1);
                    _action.canDrag = true;
                }
                break;

            case SelectedType::ENTITY_HITBOX:
                if (const auto* eh = _controller.entityHitboxController().selected()) {
                    aabb = eh->aabb();
                    _action.canDrag = true;
                }
                break;
            }

            if (_action.canDrag) {
                _action.dragAabb = aabb;

                const ms8point fm(mouseX, mouseY);

                if (selectedType == SelectedType::ENTITY_HITBOX) {

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

        const double zoomX = _controller.settings().zoomX();
        const double zoomY = _controller.settings().zoomY();

        int mouseX = std::lround((event->x - allocation.get_x()) / zoomX) - _xOffset;
        int mouseY = std::lround((event->y - allocation.get_y()) / zoomY) - _yOffset;

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

        const double zoomX = _controller.settings().zoomX();
        const double zoomY = _controller.settings().zoomY();

        int x = std::lround((event->x - allocation.get_x()) / zoomX) - _xOffset;
        int y = std::lround((event->y - allocation.get_y()) / zoomY) - _yOffset;

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
        SelectedType type = SelectedType::NONE;
        MS::FrameObject* frameObject = nullptr;
        MS::ActionPoint* actionPoint = nullptr;
        MS::EntityHitbox* entityHitbox = nullptr;
    };
    SelHandler current;
    SelHandler firstMatch;

    const MS::Frame* sFrame = _selectedFrame;
    const auto selectedType = _controller.selectedTypeController().type();

    for (MS::FrameObject& obj : sFrame->objects()) {
        const auto loc = obj.location();

        if (mouse.x >= loc.x && mouse.x < (loc.x + (int)obj.sizePx())
            && mouse.y >= loc.y && mouse.y < (loc.y + (int)obj.sizePx())) {
            if (current.type == SelectedType::NONE) {
                current.frameObject = &obj;
                current.type = SelectedType::FRAME_OBJECT;

                if (firstMatch.type == SelectedType::NONE) {
                    firstMatch.frameObject = &obj;
                    firstMatch.type = SelectedType::FRAME_OBJECT;
                }
            }

            if (&obj == _controller.frameObjectController().selected()
                && selectedType == SelectedType::FRAME_OBJECT) {

                current.type = SelectedType::NONE;
            }
        }
    }

    for (MS::ActionPoint& ap : sFrame->actionPoints()) {
        const auto loc = ap.location();

        if (mouse == loc) {
            if (current.type == SelectedType::NONE) {
                current.actionPoint = &ap;
                current.type = SelectedType::ACTION_POINT;

                if (firstMatch.type == SelectedType::NONE) {
                    firstMatch.actionPoint = &ap;
                    firstMatch.type = SelectedType::ACTION_POINT;
                }
            }
            if (&ap == _controller.actionPointController().selected()
                && selectedType == SelectedType::ACTION_POINT) {

                current.type = SelectedType::NONE;
            }
        }
    }

    for (MS::EntityHitbox& eh : sFrame->entityHitboxes()) {
        const auto aabb = eh.aabb();

        if (aabb.contains(mouse)) {
            if (current.type == SelectedType::NONE) {
                current.entityHitbox = &eh;
                current.type = SelectedType::ENTITY_HITBOX;

                if (firstMatch.type == SelectedType::NONE) {
                    firstMatch.entityHitbox = &eh;
                    firstMatch.type = SelectedType::ENTITY_HITBOX;
                }
            }
            if (&eh == _controller.entityHitboxController().selected()
                && selectedType == SelectedType::ACTION_POINT) {

                current.type = SelectedType::NONE;
            }
        }
    }

    if (current.type == SelectedType::NONE) {
        // handle wrap around.
        current = firstMatch;
    }

    switch (current.type) {
    case SelectedType::NONE:
        _controller.frameObjectController().setSelected(nullptr);
        _controller.actionPointController().setSelected(nullptr);
        _controller.entityHitboxController().setSelected(nullptr);
        break;

    case SelectedType::FRAME_OBJECT:
        _controller.frameObjectController().setSelected(current.frameObject);
        break;

    case SelectedType::ACTION_POINT:
        _controller.actionPointController().setSelected(current.actionPoint);
        break;

    case SelectedType::ENTITY_HITBOX:
        _controller.entityHitboxController().setSelected(current.entityHitbox);
        break;
    }
}

void FrameGraphicalEditor::handleRelease_Drag()
{
    _action.state = Action::NONE;
    update_pointer_cursor();

    const auto selectedType = _controller.selectedTypeController().type();
    const auto aabb = _action.dragAabb;

    switch (selectedType) {
    case SelectedType::NONE:
        break;

    case SelectedType::FRAME_OBJECT:
        _controller.frameObjectController().selected_setLocation(ms8point(aabb.x, aabb.y));
        break;

    case SelectedType::ACTION_POINT:
        _controller.actionPointController().selected_setLocation(ms8point(aabb.x, aabb.y));
        break;

    case SelectedType::ENTITY_HITBOX:
        _controller.entityHitboxController().selected_setAabb(aabb);
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

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    _xOffset = allocation.get_width() / zoomX / 2 + _centerX;
    _yOffset = allocation.get_height() / zoomY / 2 + _centerY;

    queue_draw();
}
