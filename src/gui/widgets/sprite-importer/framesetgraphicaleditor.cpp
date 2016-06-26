#include "framesetgraphicaleditor.h"
#include "gui/widgets/common/cr_rgba.h"
#include "gui/widgets/defaults.h"
#include "models/common/string.h"

#include <cmath>

using namespace UnTech::Widgets::SpriteImporter;

typedef SI::SpriteImporterController::SelectedTypeController::Type SelectedType;

FrameSetGraphicalEditor::FrameSetGraphicalEditor(SI::SpriteImporterController& controller)
    : Gtk::DrawingArea()
    , _controller(controller)
    , _frameSetImage()
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

    // Controller Signals

    _controller.settings().signal_zoomChanged().connect([this](void) {
        resizeWidget();
        loadAndScaleImage();
    });

    _controller.frameController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged));
    _controller.frameController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged)));
    _controller.frameController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged)));

    _controller.frameObjectController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged));
    _controller.frameObjectController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged)));
    _controller.frameObjectController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged)));

    _controller.entityHitboxController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged));
    _controller.entityHitboxController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged)));
    _controller.entityHitboxController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged)));

    _controller.actionPointController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged));
    _controller.actionPointController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged)));
    _controller.actionPointController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged)));

    _controller.frameSetController().signal_selectedChanged().connect(
        [this](void) {
            loadAndScaleImage();
            resizeWidget();
        });

    _controller.frameSetController().signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGraphicalEditor::on_dataChanged)));

    _controller.frameSetController().signal_imageChanged().connect(sigc::hide(
        [this](void) {
            loadAndScaleImage();
            resizeWidget();
        }));

    _controller.frameSetController().signal_selectTransparentModeChanged().connect(
        [this](void) {
            if (_controller.frameSetController().selectTransparentMode()) {
                _action.state = Action::SELECT_TRANSPARENT_COLOR;
            }
            else {
                _action.state = Action::NONE;
            }
            update_pointer_cursor();
        });
}

void FrameSetGraphicalEditor::resizeWidget()
{
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();

    if (frameSet && !frameSet->image().empty()) {
        const auto imgSize = frameSet->image().size();

        const double zoomX = _controller.settings().zoomX();
        const double zoomY = _controller.settings().zoomY();

        this->set_size_request(imgSize.width * zoomX,
                               imgSize.height * zoomY);
    }
    else {
        this->set_size_request(-1, -1);
    }

    queue_draw();
}

void FrameSetGraphicalEditor::on_dataChanged()
{
    if (_action.state != Action::NONE) {
        _action.state = Action::NONE;
        update_pointer_cursor();
    }
    queue_draw();
}

void FrameSetGraphicalEditor::loadAndScaleImage()
{
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();

    if (frameSet) {
        const auto& img = frameSet->image();

        if (!img.empty()) {
            const double zoomX = _controller.settings().zoomX();
            const double zoomY = _controller.settings().zoomY();

            int width = img.size().width * zoomX;
            int height = img.size().height * zoomY;

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

    on_dataChanged();
}

bool FrameSetGraphicalEditor::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    const double FRAME_BORDER_WIDTH = 1.0;

    const double ORIGIN_WIDTH = 1.0;
    const double ORIGIN_SIZE = 3.0;
    const double ORIGIN_DASH = 3.0;

    // ::TODO move::
    const cr_rgba frameBorderColor = { 0.5, 0.5, 0.5, 1.0 };
    const cr_rgba frameSelectedClipColor = { 0.7, 0.7, 0.7, 0.7 };
    const cr_rgba frameTileHitboxColor = { 0.8, 0.0, 0.0, 0.7 };
    const cr_rgba frameObjectColor = { 0.3, 0.9, 0.3, 0.7 };
    const cr_rgba actionPointColor = { 0.7, 1.0, 0.5, 0.95 };
    const cr_rgba entityHitboxColor = { 0.2, 0.0, 0.8, 0.7 };
    const cr_rgba selectionInnerColor = { 1.0, 1.0, 1.0, 1.0 };
    const cr_rgba selectionOuterColor = { 0.0, 0.0, 0.0, 1.0 };
    const cr_rgba selectionDragColor = { 0.5, 0.5, 0.5, 0.5 };

    const double lineWidth = _controller.settings().lineWidth();
    const double actionPointSize = 1.5 * lineWidth;

    const cr_rgba originColor1 = { 0.0, 0.0, 0.0, 0.5 };
    const cr_rgba originColor2 = { 1.0, 1.0, 1.0, 0.5 };

    const SI::FrameSet* frameSet = _controller.frameSetController().selected();

    if (frameSet == nullptr) {
        return true;
    }

    const double zoomX = _controller.settings().zoomX();
    const double zoomY = _controller.settings().zoomY();

    auto draw_rectangle = [&](unsigned x, unsigned y, unsigned width, unsigned height) {
        cr->rectangle(x * zoomX, y * zoomY,
                      width * zoomX, height * zoomY);
    };

    cr->save();
    cr->set_antialias(Cairo::ANTIALIAS_NONE);

    Gdk::Cairo::set_source_pixbuf(cr, _frameSetImage, 0, 0);
    cr->paint();

    frameBorderColor.apply(cr);
    cr->set_line_width(FRAME_BORDER_WIDTH);
    for (const auto frameIt : frameSet->frames()) {
        const SI::Frame& frame = frameIt.second;
        const auto frameLoc = frame.location();

        draw_rectangle(frameLoc.x, frameLoc.y, frameLoc.width, frameLoc.height);
        cr->stroke();
    }

    for (const auto frameIt : frameSet->frames()) {
        const SI::Frame& frame = frameIt.second;
        const auto frameLoc = frame.location();

        auto draw_frame_rectangle = [&](unsigned x, unsigned y,
                                        unsigned width, unsigned height) {
            double rw = width * zoomX - lineWidth;
            double rh = height * zoomY - lineWidth;

            // Shrink rectangle by line width so it fits **inside** the frame rectangle.
            if (x + width >= frameLoc.width) {
                rw -= lineWidth;
            }
            if (y + height >= frameLoc.height) {
                rh -= lineWidth;
            }

            cr->rectangle((frameLoc.x + x) * zoomX + lineWidth,
                          (frameLoc.y + y) * zoomY + lineWidth,
                          rw, rh);
        };

        cr->set_line_width(lineWidth);

        if (frame.solid()) {
            const auto& hb = frame.tileHitbox();

            draw_frame_rectangle(hb.x, hb.y,
                                 hb.width, hb.height);
            frameTileHitboxColor.apply(cr);
            cr->stroke();
        }

        for (const SI::EntityHitbox& eh : frame.entityHitboxes()) {
            const auto aabb = eh.aabb();

            draw_frame_rectangle(aabb.x, aabb.y,
                                 aabb.width, aabb.height);

            // ::SHOULDO different color lines depending on type::
            entityHitboxColor.apply(cr);
            cr->stroke();
        }

        for (const SI::FrameObject& obj : frame.objects()) {
            const auto oloc = obj.location();

            draw_frame_rectangle(oloc.x, oloc.y,
                                 obj.sizePx(), obj.sizePx());
            frameObjectColor.apply(cr);
            cr->stroke();
        }

        for (const SI::ActionPoint& ap : frame.actionPoints()) {
            const auto aLoc = ap.location();

            double aWidth = actionPointSize * zoomX / 2;
            double aHeight = actionPointSize * zoomY / 2;
            double x = (frameLoc.x + aLoc.x + 0.5) * zoomX;
            double y = (frameLoc.y + aLoc.y + 0.5) * zoomY;

            cr->move_to(x, y - aHeight);
            cr->line_to(x, y + aHeight);
            cr->move_to(x - aWidth, y);
            cr->line_to(x + aWidth, y);

            // ::SHOULDO different color lines depending on type::
            actionPointColor.apply(cr);
            cr->stroke();
        }
    }

    const SI::Frame* frame = _controller.frameController().selected();

    if (frame) {
        // highlight everything that is not the selected frame.
        const auto sLoc = frame->location();
        auto allocation = get_allocation();
        const unsigned aWidth = allocation.get_width();
        const unsigned aHeight = allocation.get_height();

        draw_rectangle(0, 0, sLoc.x, aHeight);
        draw_rectangle(0, 0, aWidth, sLoc.y);
        draw_rectangle(sLoc.right(), 0, aWidth - sLoc.right(), aHeight);
        draw_rectangle(0, sLoc.bottom(), aWidth, aHeight - sLoc.bottom());

        frameSelectedClipColor.apply(cr);
        cr->fill();

        // Draw selected item.
        auto draw_selected_rectangle = [&](const urect& frameLoc,
                                           unsigned x, unsigned y,
                                           unsigned width, unsigned height) {
            cr->set_line_width(1);

            const double zX = (frameLoc.x + x) * zoomX + 1;
            const double zY = (frameLoc.y + y) * zoomY + 1;
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

        switch (_controller.selectedTypeController().type()) {
        case SelectedType::NONE:
            break;

        case SelectedType::FRAME_OBJECT:
            if (const auto* fo = _controller.frameObjectController().selected()) {
                const auto oLoc = fo->location();
                const unsigned oSize = fo->sizePx();

                frameObjectColor.apply(cr);
                draw_selected_rectangle(fo->frame().location(),
                                        oLoc.x, oLoc.y, oSize, oSize);
            }
            break;

        case SelectedType::ENTITY_HITBOX:
            if (const auto* eh = _controller.entityHitboxController().selected()) {
                const auto aabb = eh->aabb();

                // ::SHOULDO different color lines depending on type::
                entityHitboxColor.apply(cr);
                draw_selected_rectangle(eh->frame().location(),
                                        aabb.x, aabb.y, aabb.width, aabb.height);
            }
            break;

        case SelectedType::ACTION_POINT:
            if (const auto* ap = _controller.actionPointController().selected()) {
                const auto frameLoc = ap->frame().location();
                const auto aLoc = ap->location();

                cr->save();

                double aWidth = actionPointSize * zoomX / 2;
                double aHeight = actionPointSize * zoomY / 2;
                double x = (frameLoc.x + aLoc.x + 0.5) * zoomX;
                double y = (frameLoc.y + aLoc.y + 0.5) * zoomY;

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

        if (_action.state == Action::DRAG) {
            const urect aabb = _action.dragAabb;
            const urect fLoc = frame->location();

            // ::SHOULDDO checkerboard pattern::

            draw_rectangle(fLoc.x + aabb.x, fLoc.y + aabb.y,
                           aabb.width, aabb.height);
            selectionDragColor.apply(cr);
            cr->fill();
        }
    }

    // Draw Origins
    static const std::vector<double> originDash({ ORIGIN_DASH, ORIGIN_DASH });

    for (const auto frameIt : frameSet->frames()) {
        const auto frameLoc = frameIt.second.location();
        const auto origin = frameIt.second.origin();

        double oWidth = ORIGIN_SIZE * zoomX / 2;
        double oHeight = ORIGIN_SIZE * zoomY / 2;
        double x = (frameLoc.x + origin.x + 0.5) * zoomX;
        double y = (frameLoc.y + origin.y + 0.5) * zoomY;

        cr->move_to(x, y - oHeight);
        cr->line_to(x, y + oHeight);
        cr->move_to(x - oWidth, y);
        cr->line_to(x + oWidth, y);
    }

    cr->set_line_width(ORIGIN_WIDTH);

    originColor1.apply(cr);
    cr->set_dash(originDash, 0.0);
    cr->stroke_preserve();

    originColor2.apply(cr);
    cr->set_dash(originDash, ORIGIN_DASH);
    cr->stroke();

    cr->restore();

    return true;
}

bool FrameSetGraphicalEditor::on_button_press_event(GdkEventButton* event)
{
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();

    if (frameSet == nullptr) {
        return false;
    }

    if (event->button == 1) {
        if (_action.state == Action::NONE) {
            const double zoomX = _controller.settings().zoomX();
            const double zoomY = _controller.settings().zoomY();

            int mouseX = std::lround(event->x / zoomX);
            int mouseY = std::lround(event->y / zoomY);

            _action.canDrag = false;

            if (mouseX >= 0 && mouseY >= 0) {
                _action.state = Action::CLICK;
                _action.pressLocation = upoint((unsigned)mouseX, (unsigned)mouseY);
                urect aabb;

                const SI::Frame* frame = _controller.frameController().selected();
                const auto selectedType = _controller.selectedTypeController().type();

                if (frame) {
                    switch (selectedType) {
                    case SelectedType::NONE:
                        break;

                    case SelectedType::FRAME_OBJECT:
                        if (const auto* fo = _controller.frameObjectController().selected()) {
                            const auto foLoc = fo->location();

                            aabb = urect(foLoc.x, foLoc.y, fo->sizePx(), fo->sizePx());
                            _action.canDrag = true;
                        }
                        break;

                    case SelectedType::ACTION_POINT:
                        if (const auto* ap = _controller.actionPointController().selected()) {
                            const auto apLoc = ap->location();

                            aabb = urect(apLoc.x, apLoc.y, 1, 1);
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

                        const auto fLoc = frame->location();
                        const upoint fm(mouseX - fLoc.x, mouseY - fLoc.y);

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
        }
    }
    return false;
}

bool FrameSetGraphicalEditor::on_motion_notify_event(GdkEventMotion* event)
{
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();
    const SI::Frame* frame = _controller.frameController().selected();

    if (frameSet == nullptr) {
        _action.state = Action::NONE;
        return true;
    }

    if (frame && _action.state != Action::NONE) {
        const double zoomX = _controller.settings().zoomX();
        const double zoomY = _controller.settings().zoomY();

        auto allocation = get_allocation();

        int mouseX = std::lround((event->x - allocation.get_x()) / zoomX);
        int mouseY = std::lround((event->y - allocation.get_y()) / zoomY);

        if (mouseX >= 0 && mouseY >= 0) {
            upoint mouse((unsigned)mouseX, (unsigned)mouseY);

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
                    urect aabb = _action.dragAabb;

                    if (!_action.resize) {
                        // move
                        int dx = mouseX - _action.previousLocation.x;
                        int dy = mouseY - _action.previousLocation.y;

                        // handle underflow
                        if (dx >= 0 || aabb.x > (unsigned)(-dx)) {
                            aabb.x += dx;
                        }
                        else {
                            aabb.x = 0;
                        }
                        if (dy >= 0 || aabb.y > (unsigned)(-dy)) {
                            aabb.y += dy;
                        }
                        else {
                            aabb.y = 0;
                        }
                    }
                    else {
                        // resize
                        const auto& fLoc = frame->location();
                        int fmx = mouse.x - fLoc.x;
                        if (fmx < 0) {
                            fmx = 0;
                        }
                        if (_action.resizeLeft) {
                            if ((unsigned)fmx >= aabb.right()) {
                                fmx = aabb.right() - 1;
                            }
                            aabb.width = aabb.right() - fmx;
                            aabb.x = fmx;
                        }
                        else if (_action.resizeRight) {
                            if ((unsigned)fmx <= aabb.left()) {
                                fmx = aabb.left() + 1;
                            }
                            aabb.width = fmx - aabb.x;
                        }

                        int fmy = mouse.y - fLoc.y;
                        if (fmy < 0) {
                            fmy = 0;
                        }
                        if (_action.resizeTop) {
                            if ((unsigned)fmy >= aabb.bottom()) {
                                fmy = aabb.bottom() - 1;
                            }
                            aabb.height = aabb.bottom() - fmy;
                            aabb.y = fmy;
                        }
                        else if (_action.resizeBottom) {
                            if ((unsigned)fmy <= aabb.top()) {
                                fmy = aabb.top() + 1;
                            }
                            aabb.height = fmy - aabb.y;
                        }
                    }

                    aabb = frame->location().clipInside(aabb, _action.dragAabb);

                    if (_action.dragAabb != aabb) {
                        _action.dragAabb = aabb;
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
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();

    grab_focus();

    if (frameSet == nullptr) {
        return false;
    }

    if (event->button == 1) {
        const double zoomX = _controller.settings().zoomX();
        const double zoomY = _controller.settings().zoomY();

        int x = std::lround(event->x / zoomX);
        int y = std::lround(event->y / zoomY);

        if (x >= 0 && y >= 0) {
            upoint mouse((unsigned)x, (unsigned)y);

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
    return false;
}

void FrameSetGraphicalEditor::handleRelease_Click(const upoint& mouse)
{
    _action.state = Action::NONE;

    // only select item if mouse didn't move.
    if (mouse != _action.pressLocation) {
        return;
    }

    const SI::Frame* sFrame = _controller.frameController().selected();

    if (sFrame && sFrame->location().contains(mouse)) {
        const auto& sFrameLoc = sFrame->location();
        const upoint frameMouse(mouse.x - sFrameLoc.x, mouse.y - sFrameLoc.y);

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
            SI::FrameObject* frameObject = nullptr;
            SI::ActionPoint* actionPoint = nullptr;
            SI::EntityHitbox* entityHitbox = nullptr;
        };
        SelHandler current;
        SelHandler firstMatch;

        const auto selectedType = _controller.selectedTypeController().type();

        for (SI::FrameObject& obj : sFrame->objects()) {
            const auto& loc = obj.location();

            if (frameMouse.x >= loc.x && frameMouse.x < (loc.x + obj.sizePx())
                && frameMouse.y >= loc.y && frameMouse.y < (loc.y + obj.sizePx())) {
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

        for (SI::ActionPoint& ap : sFrame->actionPoints()) {
            const auto& loc = ap.location();

            if (frameMouse == loc) {
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

        for (SI::EntityHitbox& eh : sFrame->entityHitboxes()) {
            const auto& aabb = eh.aabb();

            if (aabb.contains(frameMouse)) {
                if (current.type == SelectedType::NONE) {
                    current.entityHitbox = &eh;
                    current.type = SelectedType::ENTITY_HITBOX;

                    if (firstMatch.type == SelectedType::NONE) {
                        firstMatch.entityHitbox = &eh;
                        firstMatch.type = SelectedType::ENTITY_HITBOX;
                    }
                }
                if (&eh == _controller.entityHitboxController().selected()
                    && selectedType == SelectedType::ENTITY_HITBOX) {

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
    else {
        const SI::FrameSet* frameSet = _controller.frameSetController().selected();

        if (frameSet) {
            for (const auto fIt : frameSet->frames()) {
                const auto frameLoc = fIt.second.location();

                if (frameLoc.contains(mouse)) {
                    _controller.frameController().setSelected(&fIt.second);
                    return;
                }
            }
        }

        // click is not inside a frame
        // unselect current frame.
        _controller.frameController().setSelected(nullptr);
    }
}

void FrameSetGraphicalEditor::handleRelease_SelectTransparentColor(const upoint& mouse)
{
    const SI::FrameSet* frameSet = _controller.frameSetController().selected();

    if (frameSet) {
        const auto& image = frameSet->image();
        if (!image.empty()) {
            auto size = image.size();
            if (mouse.x < size.width && mouse.y < size.height) {
                auto color = frameSet->image().getPixel(mouse.x, mouse.y);

                _controller.frameSetController().selected_setTransparentColor(color);
            }
        }
    }

    _action.state = Action::NONE;
    update_pointer_cursor();

    _controller.frameSetController().setSelectTransparentMode(false);
}

void FrameSetGraphicalEditor::handleRelease_Drag()
{
    _action.state = Action::NONE;
    update_pointer_cursor();

    const auto aabb = _action.dragAabb;

    switch (_controller.selectedTypeController().type()) {
    case SelectedType::NONE:
        break;

    case SelectedType::FRAME_OBJECT:
        _controller.frameObjectController().selected_setLocation(upoint(aabb.x, aabb.y));
        break;

    case SelectedType::ACTION_POINT:
        _controller.actionPointController().selected_setLocation(upoint(aabb.x, aabb.y));
        break;

    case SelectedType::ENTITY_HITBOX:
        _controller.entityHitboxController().selected_setAabb(aabb);
        break;
    }

    queue_draw();
}

bool FrameSetGraphicalEditor::on_enter_notify_event(GdkEventCrossing*)
{
    update_pointer_cursor();
    return true;
}

void FrameSetGraphicalEditor::update_pointer_cursor()
{
    auto win = get_window();
    if (win) {
        switch (_action.state) {
        case Action::NONE:
        case Action::CLICK:
            win->set_cursor();
            break;

        case Action::SELECT_TRANSPARENT_COLOR:
            win->set_cursor(Gdk::Cursor::create(get_display(), "cell"));
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

bool FrameSetGraphicalEditor::on_leave_notify_event(GdkEventCrossing*)
{
    auto win = get_window();
    if (win) {
        win->set_cursor();
    }

    return true;
}
