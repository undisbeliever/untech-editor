#include "frameeditor.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

FrameEditor::FrameEditor()
    : widget()
    , _selectedFrame(nullptr)
    , _frameParameterEditor()
    , _frameObjectBox(Gtk::ORIENTATION_VERTICAL)
    , _frameObjectList()
    , _frameObjectEditor()
    , _actionPointBox(Gtk::ORIENTATION_VERTICAL)
    , _actionPointList()
    , _actionPointEditor()
    , _entityHitboxBox(Gtk::ORIENTATION_VERTICAL)
    , _entityHitboxList()
    , _entityHitboxEditor()
{
    widget.set_scrollable(true);
    widget.popup_enable();

    widget.append_page(_frameParameterEditor.widget, _("Frame"));

    _frameObjectBox.set_border_width(DEFAULT_BORDER);
    _frameObjectBox.pack_start(_frameObjectList.widget, Gtk::PACK_EXPAND_WIDGET);
    _frameObjectBox.pack_start(_frameObjectEditor.widget, Gtk::PACK_SHRINK);
    widget.append_page(_frameObjectBox, _("Objects"));

    _actionPointBox.set_border_width(DEFAULT_BORDER);
    _actionPointBox.pack_start(_actionPointList.widget, Gtk::PACK_EXPAND_WIDGET);
    _actionPointBox.pack_start(_actionPointEditor.widget, Gtk::PACK_SHRINK);
    widget.append_page(_actionPointBox, _("Action Points"));

    _entityHitboxBox.set_border_width(DEFAULT_BORDER);
    _entityHitboxBox.pack_start(_entityHitboxList.widget, Gtk::PACK_EXPAND_WIDGET);
    _entityHitboxBox.pack_start(_entityHitboxEditor.widget, Gtk::PACK_SHRINK);
    widget.append_page(_entityHitboxBox, _("Entity Hitboxes"));

    /*
     * SLOTS
     * =====
     */
    _frameObjectList.signal_selected_changed().connect([this](void) {
        setFrameObject(_frameObjectList.getSelected());
    });
    _actionPointList.signal_selected_changed().connect([this](void) {
        setActionPoint(_actionPointList.getSelected());
    });
    _entityHitboxList.signal_selected_changed().connect([this](void) {
        setEntityHitbox(_entityHitboxList.getSelected());
    });
}

void FrameEditor::setFrame(std::shared_ptr<SI::Frame> frame)
{
    if (_selectedFrame != frame) {
        _selectedFrame = frame;

        _frameParameterEditor.setFrame(frame);

        if (frame) {
            _frameObjectList.setList(frame->objects());
            _actionPointList.setList(frame->actionPoints());
            _entityHitboxList.setList(frame->entityHitboxes());
            widget.set_sensitive(true);
        }
        else {
            _frameObjectList.setList(nullptr);
            _actionPointList.setList(nullptr);
            _entityHitboxList.setList(nullptr);
            widget.set_sensitive(false);
        }

        setFrameObject(nullptr);
        setActionPoint(nullptr);
        setEntityHitbox(nullptr);
    }
}

void FrameEditor::setFrameObject(std::shared_ptr<SI::FrameObject> frameObject)
{
    if (frameObject) {
        setFrame(frameObject->frame().lock());
        widget.set_current_page(FRAME_OBJECT_PAGE);
    }

    _frameObjectList.selectItem(frameObject);
    _frameObjectEditor.setFrameObject(frameObject);
}

void FrameEditor::setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint)
{
    if (actionPoint) {
        setFrame(actionPoint->frame().lock());
        widget.set_current_page(ACTION_POINT_PAGE);
    }

    _actionPointList.selectItem(actionPoint);
    _actionPointEditor.setActionPoint(actionPoint);
}

void FrameEditor::setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox)
{
    if (entityHitbox) {
        setFrame(entityHitbox->frame().lock());
        widget.set_current_page(ENTITY_HITBOX_PAGE);
    }

    _entityHitboxList.selectItem(entityHitbox);
    _entityHitboxEditor.setEntityHitbox(entityHitbox);
}
