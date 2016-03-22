#include "framesetgraphicaleditor.h"
#include "signals.h"
#include "../common/cr_rgba.h"

#include <cmath>

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

FrameSetGraphicalEditor::FrameSetGraphicalEditor()
    : Gtk::DrawingArea()
    , _frameSet(nullptr)
    , _selectedFrame(nullptr)
    , _zoomX(3.0)
    , _zoomY(3.0)
    , _displayZoom(NAN)
    , _frameSetImage()
    , _selection()
{
    add_events(Gdk::BUTTON_PRESS_MASK
               | Gdk::BUTTON_RELEASE_MASK
               | Gdk::ENTER_NOTIFY_MASK
               | Gdk::LEAVE_NOTIFY_MASK
               | Gdk::BUTTON1_MOTION_MASK);

    // SLOTS
    // =====
    Signals::frameListChanged.connect(sigc::hide(sigc::mem_fun(this, &FrameSetGraphicalEditor::queue_draw)));
    Signals::frameObjectListChanged.connect(sigc::hide(sigc::mem_fun(this, &FrameSetGraphicalEditor::queue_draw)));
    Signals::actionPointListChanged.connect(sigc::hide(sigc::mem_fun(this, &FrameSetGraphicalEditor::queue_draw)));
    Signals::entityHitboxListChanged.connect(sigc::hide(sigc::mem_fun(this, &FrameSetGraphicalEditor::queue_draw)));

    Signals::frameSetGridChanged.connect([this](const std::shared_ptr<SI::FrameSet> frameSet) {
        if (frameSet == _frameSet) {
            queue_draw();
        }
    });
    Signals::frameSizeChanged.connect([this](const std::shared_ptr<SI::Frame> frame) {
        if (frame && frame->frameSet() == _frameSet) {
            queue_draw();
        }
    });
    Signals::frameObjectChanged.connect([this](const std::shared_ptr<SI::FrameObject> obj) {
        if (obj && obj->frame()->frameSet() == _frameSet) {
            queue_draw();
        }
    });
    Signals::actionPointLocationChanged.connect([this](const std::shared_ptr<SI::ActionPoint> ap) {
        if (ap && ap->frame()->frameSet() == _frameSet) {
            queue_draw();
        }
    });
    Signals::entityHitboxLocationChanged.connect([this](const std::shared_ptr<SI::EntityHitbox> eh) {
        if (eh && eh->frame()->frameSet() == _frameSet) {
            queue_draw();
        }
    });
}

// ::TODO call on signal_frameSetImageChanged signal::
void FrameSetGraphicalEditor::resizeWidget()
{
    if (_frameSet && !_frameSet->image().empty() && _displayZoom > 0.0) {
        const auto imgSize = _frameSet->image().size();

        this->set_size_request(imgSize.width * _zoomX * _displayZoom,
                               imgSize.height * _zoomY * _displayZoom);
    }
    else {
        this->set_size_request(-1, -1);
    }

    queue_draw();
}

void FrameSetGraphicalEditor::loadAndScaleImage()
{
    if (_frameSet) {
        const auto& img = _frameSet->image();

        if (!img.empty()) {
            int width = img.size().width * _zoomX;
            int height = img.size().height * _zoomY;

            auto pixbuf = Gdk::Pixbuf::create_from_data(reinterpret_cast<const guint8*>(img.data()),
                                                        Gdk::Colorspace::COLORSPACE_RGB, true, 8,
                                                        img.size().width, img.size().height,
                                                        img.size().width * 4);

            // Scaling is done by GTK not Cairo, as it results in sharp pixels
            _frameSetImage = pixbuf->scale_simple(width, height, Gdk::InterpType::INTERP_NEAREST);
        }
        else {
            // show a gray tile
            _frameSetImage = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, 16, 16);
            _frameSetImage->fill(0x80808080);
        }
    }
    else {
        // show an empty pixel
        _frameSetImage = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, 1, 1);
        _frameSetImage->fill(0);
    }

    queue_draw();
}

bool FrameSetGraphicalEditor::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // ::TODO move::
    const double FRAME_BORDER_WIDTH = 1.0;
    const double ITEM_WIDTH = 1.0;
    const double ACTION_POINT_SIZE = 1.5;
    const cr_rgba frameBorderColor = { 0.5, 0.5, 0.5, 1.0 };
    const cr_rgba frameSelectedClipColor = { 0.7, 0.7, 0.7, 0.7 };
    const cr_rgba frameTileHitboxColor = { 0.8, 0.0, 0.0, 0.7 };
    const cr_rgba frameObjectColor = { 0.3, 0.9, 0.3, 0.7 };
    const cr_rgba actionPointColor = { 0.7, 1.0, 0.5, 0.95 };
    const cr_rgba entityHitboxColor = { 0.2, 0.0, 0.8, 0.7 };
    const cr_rgba selectionInnerColor = { 1.0, 1.0, 1.0, 1.0 };
    const cr_rgba selectionOuterColor = { 0.0, 0.0, 0.0, 1.0 };
    const cr_rgba selectionDragColor = { 0.5, 0.5, 0.5, 0.5 };

    if (_frameSet == nullptr) {
        return true;
    }

    if (std::isnan(_displayZoom)) {
        auto screen = get_screen();

        if (screen) {
            _displayZoom = std::floor(screen->get_width() / 1500) + 1.0;
            resizeWidget();
        }
        else {
            _displayZoom = 1.0;
        }
    }

    auto draw_rectangle = [this, cr](unsigned x, unsigned y, unsigned width, unsigned height) {
        cr->rectangle(x * _zoomX, y * _zoomY,
                      width * _zoomX, height * _zoomY);
    };

    cr->save();
    cr->set_antialias(Cairo::ANTIALIAS_NONE);
    cr->scale(_displayZoom, _displayZoom);

    Gdk::Cairo::set_source_pixbuf(cr, _frameSetImage, 0, 0);
    cr->paint();

    if (_displayZoom > 1.0) {
        cr->set_antialias(Cairo::ANTIALIAS_DEFAULT);
    }

    frameBorderColor.apply(cr);
    cr->set_line_width(FRAME_BORDER_WIDTH);
    for (const auto frameIt : _frameSet->frames()) {
        const auto frame = frameIt.second;
        const auto frameLoc = frame->location();

        draw_rectangle(frameLoc.x, frameLoc.y, frameLoc.width, frameLoc.height);
        cr->stroke();
    }

    for (const auto frameIt : _frameSet->frames()) {
        const auto frame = frameIt.second;
        const auto frameLoc = frame->location();

        auto draw_frame_rectangle = [&](unsigned x, unsigned y,
                                        unsigned width, unsigned height) {
            double rw = width * _zoomX - ITEM_WIDTH;
            double rh = height * _zoomY - ITEM_WIDTH;

            // Shrink rectangle 1px so it fits **inside** the frame rectangle.
            if (x + width >= frameLoc.width) {
                rw -= ITEM_WIDTH;
            }
            if (y + height >= frameLoc.height) {
                rh -= ITEM_WIDTH;
            }

            cr->rectangle((frameLoc.x + x) * _zoomX + ITEM_WIDTH,
                          (frameLoc.y + y) * _zoomY + ITEM_WIDTH,
                          rw, rh);
        };

        cr->set_line_width(ITEM_WIDTH);

        if (frame->solid()) {
            const auto& hb = frame->tileHitbox();

            draw_frame_rectangle(hb.x, hb.y,
                                 hb.width, hb.height);
            frameTileHitboxColor.apply(cr);
            cr->stroke();
        }

        for (const auto eh : frame->entityHitboxes()) {
            const auto aabb = eh->aabb();

            draw_frame_rectangle(aabb.x, aabb.y,
                                 aabb.width, aabb.height);

            // ::SHOULDO different color lines depending on type::
            entityHitboxColor.apply(cr);
            cr->stroke();
        }

        for (const auto obj : frame->objects()) {
            const auto oloc = obj->location();

            draw_frame_rectangle(oloc.x, oloc.y,
                                 obj->sizePx(), obj->sizePx());
            frameObjectColor.apply(cr);
            cr->stroke();
        }

        for (const auto ap : frame->actionPoints()) {
            const auto aLoc = ap->location();

            double aWidth = ACTION_POINT_SIZE * _zoomX / 2;
            double aHeight = ACTION_POINT_SIZE * _zoomY / 2;
            double x = (frameLoc.x + aLoc.x + 0.5) * _zoomX;
            double y = (frameLoc.y + aLoc.y + 0.5) * _zoomY;

            cr->move_to(x, y - aHeight);
            cr->line_to(x, y + aHeight);
            cr->move_to(x - aWidth, y);
            cr->line_to(x + aWidth, y);

            // ::SHOULDO different color lines depending on type::
            actionPointColor.apply(cr);
            cr->stroke();
        }
    }

    if (_selectedFrame) {
        // highlight everything that is not the selected frame.
        const auto sLoc = _selectedFrame->location();
        auto allocation = get_allocation();
        const unsigned aWidth = allocation.get_width();
        const unsigned aHeight = allocation.get_height();

        draw_rectangle(0, 0, sLoc.x, aHeight);
        draw_rectangle(0, 0, aWidth, sLoc.y);
        draw_rectangle(sLoc.right(), 0, aWidth - sLoc.right(), aHeight);
        draw_rectangle(0, sLoc.bottom(), aWidth, aHeight - sLoc.bottom());

        frameSelectedClipColor.apply(cr);
        cr->fill();
    }

    auto draw_selected_rectangle = [&](const urect& frameLoc,
                                       unsigned x, unsigned y,
                                       unsigned width, unsigned height) {
        cr->set_line_width(1);

        const double zX = (frameLoc.x + x) * _zoomX + 1;
        const double zY = (frameLoc.y + y) * _zoomY + 1;
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

    switch (_selection.type) {
    case Selection::Type::NONE:
        break;

    case Selection::Type::FRAME_OBJECT:
        if (_selection.frameObject) {
            const auto frame = _selection.frameObject->frame();
            if (frame) {
                const auto oLoc = _selection.frameObject->location();
                const unsigned oSize = _selection.frameObject->sizePx();

                frameObjectColor.apply(cr);
                draw_selected_rectangle(frame->location(),
                                        oLoc.x, oLoc.y, oSize, oSize);
            }
        }
        break;

    case Selection::Type::ENTITY_HITBOX:
        if (_selection.entityHitbox) {
            const auto frame = _selection.entityHitbox->frame();
            if (frame) {
                const auto aabb = _selection.entityHitbox->aabb();

                // ::SHOULDO different color lines depending on type::
                entityHitboxColor.apply(cr);
                draw_selected_rectangle(frame->location(),
                                        aabb.x, aabb.y, aabb.width, aabb.height);
            }
        }
        break;

    case Selection::Type::ACTION_POINT:
        if (_selection.actionPoint) {
            const auto frame = _selection.actionPoint->frame();
            if (frame) {
                const auto frameLoc = frame->location();
                const auto aLoc = _selection.actionPoint->location();

                cr->save();

                double aWidth = ACTION_POINT_SIZE * _zoomX / 2;
                double aHeight = ACTION_POINT_SIZE * _zoomY / 2;
                double x = (frameLoc.x + aLoc.x + 0.5) * _zoomX;
                double y = (frameLoc.y + aLoc.y + 0.5) * _zoomY;

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
        }
        break;
    }

    if (_action.state == Action::DRAG) {
        const urect aabb = _action.dragAabb;
        const urect fLoc = _selectedFrame->location();

        // ::SHOULDDO checkerboard pattern::

        draw_rectangle(fLoc.x + aabb.x, fLoc.y + aabb.y,
                       aabb.width, aabb.height);
        selectionDragColor.apply(cr);
        cr->fill();
    }

    cr->restore();

    return true;
}

bool FrameSetGraphicalEditor::on_button_press_event(GdkEventButton* event)
{
    if (_frameSet == nullptr) {
        return false;
    }

    _action.canDrag = false;

    if (event->button == 1) {
        if (_action.state == Action::NONE) {
            int x = std::lround(event->x / (_zoomX * _displayZoom));
            int y = std::lround(event->y / (_zoomY * _displayZoom));

            if (x >= 0 && y >= 0) {
                _action.state = Action::CLICK;
                _action.pressLocation = { (unsigned)x, (unsigned)y };
                urect aabb;

                switch (_selection.type) {
                case Selection::Type::NONE:
                    break;

                case Selection::Type::FRAME_OBJECT:
                    if (_selection.frameObject) {
                        const auto fo = _selection.frameObject;

                        if (fo->frame()) {
                            const auto foLoc = fo->location();
                            aabb = { foLoc.x, foLoc.y, fo->sizePx(), fo->sizePx() };
                            _action.canDrag = true;
                        }
                    }
                    break;

                case Selection::Type::ACTION_POINT:
                    if (_selection.actionPoint) {
                        const auto ap = _selection.actionPoint;

                        if (ap->frame()) {
                            const auto apLoc = ap->location();
                            aabb = { apLoc.x, apLoc.y, 1, 1 };
                            _action.canDrag = true;
                        }
                    }
                    break;

                case Selection::Type::ENTITY_HITBOX:
                    if (_selection.entityHitbox) {
                        const auto eh = _selection.entityHitbox;

                        if (eh->frame()) {
                            aabb = eh->aabb();
                            _action.canDrag = true;
                        }
                    }
                    break;
                }

                if (_action.canDrag) {
                    _action.dragAabb = aabb;
                }
            }
        }
    }
    return true;
}

bool FrameSetGraphicalEditor::on_motion_notify_event(GdkEventMotion* event)
{
    if (_selectedFrame == nullptr) {
        _action.state = Action::NONE;
        return true;
    }

    if (_action.state != Action::NONE) {
        auto allocation = get_allocation();

        int x = std::lround((event->x - allocation.get_x()) / (_zoomX * _displayZoom));
        int y = std::lround((event->y - allocation.get_y()) / (_zoomY * _displayZoom));

        if (x >= 0 && y >= 0) {
            upoint mouse = { (unsigned)x, (unsigned)y };

            if (_action.state == Action::CLICK && _action.canDrag) {
                if (_action.pressLocation != mouse) {
                    _action.state = Action::DRAG;
                    _action.previousLocation = _action.pressLocation;

                    set_cursor_for_state(_action.state);
                }
            }

            if (_action.state == Action::DRAG) {
                // move dragAbbb to new location.
                if (_action.previousLocation != mouse) {
                    int dx = mouse.x - _action.previousLocation.x;
                    int dy = mouse.y - _action.previousLocation.y;

                    urect newAabb = _action.dragAabb;

                    // handle underflow
                    if (dx >= 0 || newAabb.x > (unsigned)(-dx)) {
                        newAabb.x += dx;
                    }
                    else {
                        newAabb.x = 0;
                    }
                    if (dy >= 0 || newAabb.y > (unsigned)(-dy)) {
                        newAabb.y += dy;
                    }
                    else {
                        newAabb.y = 0;
                    }

                    newAabb = _selectedFrame->location().clipInside(newAabb, _action.dragAabb);

                    if (_action.dragAabb != newAabb) {
                        _action.dragAabb = newAabb;
                        queue_draw();
                    }

                    _action.previousLocation = mouse;
                }
            }
        }
    }
    return true;
}

bool FrameSetGraphicalEditor::on_button_release_event(GdkEventButton* event)
{
    if (_frameSet == nullptr) {
        return false;
    }

    if (event->button == 1) {
        int x = std::lround(event->x / (_zoomX * _displayZoom));
        int y = std::lround(event->y / (_zoomY * _displayZoom));

        if (x >= 0 && y >= 0) {
            upoint mouse = { (unsigned)x, (unsigned)y };

            switch (_action.state) {
            case Action::CLICK:
                handleRelease_Click(mouse);
                break;

            case Action::SELECT_TRANSPARENT_COLOR:
                handleRelease_SelectTransparentColor(mouse);
                break;

            case Action::DRAG:
                handleRelease_Drag();
                break;

            case Action::NONE:
                break;
            }
        }
        else {
            _action.state = Action::NONE;
        }
    }
    return true;
}

void FrameSetGraphicalEditor::handleRelease_Click(const upoint& mouse)
{
    _action.state = Action::NONE;

    // only select item if mouse didn't move.
    if (mouse != _action.pressLocation) {
        return;
    }

    if (_selectedFrame && _selectedFrame->location().contains(mouse)) {
        const auto frameLoc = _selectedFrame->location();
        const upoint frameMouse = { mouse.x - frameLoc.x, mouse.y - frameLoc.y };

        /*
         * Select a given item.
         *
         * This code cycles through the given selections.
         * On click, the next item is selected. If the last item
         * was the previously selected one then the first match
         * is selected.
         */
        Selection selection;
        Selection firstMatch;

        for (const auto obj : _selectedFrame->objects()) {
            const auto loc = obj->location();

            if (frameMouse.x >= loc.x && frameMouse.x < (loc.x + obj->sizePx())
                && frameMouse.y >= loc.y && frameMouse.y < (loc.y + obj->sizePx())) {
                if (selection.type == Selection::Type::NONE) {
                    selection.frameObject = obj;
                    selection.type = Selection::Type::FRAME_OBJECT;

                    if (firstMatch.type == Selection::Type::NONE) {
                        firstMatch.frameObject = obj;
                        firstMatch.type = Selection::Type::FRAME_OBJECT;
                    }
                }
                if (obj == _selection.frameObject) {
                    selection.type = Selection::Type::NONE;
                }
            }
        }

        for (const auto ap : _selectedFrame->actionPoints()) {
            const auto loc = ap->location();

            if (frameMouse == loc) {
                if (selection.type == Selection::Type::NONE) {
                    selection.actionPoint = ap;
                    selection.type = Selection::Type::ACTION_POINT;

                    if (firstMatch.type == Selection::Type::NONE) {
                        firstMatch.actionPoint = ap;
                        firstMatch.type = Selection::Type::ACTION_POINT;
                    }
                }
                if (ap == _selection.actionPoint) {
                    selection.type = Selection::Type::NONE;
                }
            }
        }

        for (const auto eh : _selectedFrame->entityHitboxes()) {
            const auto aabb = eh->aabb();

            if (aabb.contains(frameMouse)) {
                if (selection.type == Selection::Type::NONE) {
                    selection.entityHitbox = eh;
                    selection.type = Selection::Type::ENTITY_HITBOX;

                    if (firstMatch.type == Selection::Type::NONE) {
                        firstMatch.entityHitbox = eh;
                        firstMatch.type = Selection::Type::ENTITY_HITBOX;
                    }
                }
                if (eh == _selection.entityHitbox) {
                    selection.type = Selection::Type::NONE;
                }
            }
        }

        if (selection.type == Selection::Type::NONE) {
            // handle wrap around.
            selection = firstMatch;
        }

        switch (selection.type) {
        case Selection::Type::NONE:
            unselectAll();
            break;

        case Selection::Type::FRAME_OBJECT:
            setFrameObject(selection.frameObject);
            signal_selectFrameObject.emit(selection.frameObject);
            break;

        case Selection::Type::ACTION_POINT:
            setActionPoint(selection.actionPoint);
            signal_selectActionPoint.emit(selection.actionPoint);
            break;

        case Selection::Type::ENTITY_HITBOX:
            setEntityHitbox(selection.entityHitbox);
            signal_selectEntityHitbox.emit(selection.entityHitbox);
            break;
        }

        return;
    }
    else {
        for (const auto fIt : _frameSet->frames()) {
            const auto frameLoc = fIt.second->location();

            if (frameLoc.contains(mouse)) {
                setFrame(fIt.second);
                signal_selectFrame.emit(fIt.second);
                return;
            }
        }

        // click is not inside a frame
        // unselect current frame.
        setFrame(nullptr);
        signal_selectFrame.emit(nullptr);
    }
}

void FrameSetGraphicalEditor::handleRelease_SelectTransparentColor(const upoint& mouse)
{
    _action.state = Action::NONE;
    set_cursor_for_state(Action::NONE);

    const auto& image = _frameSet->image();
    if (!image.empty()) {
        auto size = image.size();
        if (mouse.x < size.width && mouse.y < size.height) {
            auto color = _frameSet->image().getPixel(mouse.x, mouse.y);
            _frameSet->setTransparentColor(color);

            Signals::frameSetChanged.emit(_frameSet);
        }
    }
}

void FrameSetGraphicalEditor::handleRelease_Drag()
{
    _action.state = Action::NONE;
    set_cursor_for_state(Action::NONE);

    const auto aabb = _action.dragAabb;

    switch (_selection.type) {
    case Selection::Type::NONE:
        break;

    case Selection::Type::FRAME_OBJECT:
        if (_selection.frameObject) {
            _selection.frameObject->setLocation({ aabb.x, aabb.y });
            Signals::frameObjectChanged(_selection.frameObject);
        }
        break;

    case Selection::Type::ACTION_POINT:
        if (_selection.actionPoint) {
            _selection.actionPoint->setLocation({ aabb.x, aabb.y });
            Signals::actionPointChanged(_selection.actionPoint);
        }
        break;

    case Selection::Type::ENTITY_HITBOX:
        if (_selection.entityHitbox) {
            _selection.entityHitbox->setAabb(aabb);
            Signals::entityHitboxChanged(_selection.entityHitbox);
        }
        break;
    }

    queue_draw();
}

bool FrameSetGraphicalEditor::on_enter_notify_event(GdkEventCrossing*)
{
    set_cursor_for_state(_action.state);
    return true;
}

void FrameSetGraphicalEditor::set_cursor_for_state(Action::State state)
{
    auto win = get_window();
    if (win) {
        switch (state) {
        case Action::NONE:
        case Action::CLICK:
            win->set_cursor();
            break;

        case Action::SELECT_TRANSPARENT_COLOR:
            win->set_cursor(Gdk::Cursor::create(get_display(), "cell"));
            break;

        case Action::DRAG:
            win->set_cursor(Gdk::Cursor::create(get_display(), "move"));
            break;
        }
    }
}

bool FrameSetGraphicalEditor::on_leave_notify_event(GdkEventCrossing*)
{
    auto win = get_window();
    if (win) {
        win->set_cursor();
    }

    return true;
}

void FrameSetGraphicalEditor::setFrameSet(std::shared_ptr<SI::FrameSet> frameSet)
{
    if (_frameSet != frameSet) {
        _frameSet = frameSet;
        _selectedFrame = nullptr;
        unselectAll();

        loadAndScaleImage();
        resizeWidget();
    }
}

void FrameSetGraphicalEditor::setFrame(std::shared_ptr<SI::Frame> frame)
{
    if (_selectedFrame != frame) {
        _selectedFrame = frame;
        unselectAll();

        queue_draw();
    }
}

void FrameSetGraphicalEditor::setFrameObject(std::shared_ptr<SI::FrameObject> frameObject)
{
    _selection.type = Selection::Type::FRAME_OBJECT;
    _selection.frameObject = frameObject;
    _selection.actionPoint = nullptr;
    _selection.entityHitbox = nullptr;

    _action.state = Action::NONE;

    queue_draw();
}

void FrameSetGraphicalEditor::setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint)
{
    _selection.type = Selection::Type::ACTION_POINT;
    _selection.frameObject = nullptr;
    _selection.actionPoint = actionPoint;
    _selection.entityHitbox = nullptr;

    _action.state = Action::NONE;

    queue_draw();
}

void FrameSetGraphicalEditor::setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox)
{
    _selection.type = Selection::Type::ENTITY_HITBOX;
    _selection.frameObject = nullptr;
    _selection.actionPoint = nullptr;
    _selection.entityHitbox = entityHitbox;

    _action.state = Action::NONE;

    queue_draw();
}

void FrameSetGraphicalEditor::unselectAll()
{
    _selection.type = Selection::Type::NONE;
    _selection.frameObject = nullptr;
    _selection.actionPoint = nullptr;
    _selection.entityHitbox = nullptr;

    _action.state = Action::NONE;

    queue_draw();
}

void FrameSetGraphicalEditor::enableSelectTransparentColor()
{
    _action.state = Action::SELECT_TRANSPARENT_COLOR;
    set_cursor_for_state(_action.state);
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

void FrameSetGraphicalEditor::setZoom(double x, double y)
{
    if (_zoomX != x || _zoomY != y) {
        _zoomX = limit(x, 1.0, 10.0);
        _zoomY = limit(y, 1.0, 10.0);

        resizeWidget();
        loadAndScaleImage();
    }
}
