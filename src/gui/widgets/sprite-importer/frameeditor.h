#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMEDITOR_H_

#include "actionpointeditor.h"
#include "actionpointlist.h"
#include "entityhitboxeditor.h"
#include "entityhitboxlist.h"
#include "frameobjecteditor.h"
#include "frameobjectlist.h"
#include "framepropertieseditor.h"
#include "models/sprite-importer.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class FrameEditor {
public:
    FrameEditor()
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

    void setFrame(std::shared_ptr<SI::Frame> frame)
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
                widget.set_sensitive(false);
            }
        }
    }

    void setFrameObject(std::shared_ptr<SI::FrameObject> frameObject)
    {
        if (frameObject) {
            setFrame(frameObject->frame().lock());
            widget.set_current_page(FRAME_OBJECT_PAGE);
        }

        _frameObjectEditor.setFrameObject(frameObject);
    }

    void setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint)
    {
        if (actionPoint) {
            setFrame(actionPoint->frame().lock());
            widget.set_current_page(ACTION_POINT_PAGE);
        }

        _actionPointEditor.setActionPoint(actionPoint);
    }

    void setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox)
    {
        if (entityHitbox) {
            setFrame(entityHitbox->frame().lock());
            widget.set_current_page(ENTITY_HITBOX_PAGE);
        }

        _entityHitboxEditor.setEntityHitbox(entityHitbox);
    }

public:
    Gtk::Notebook widget;

private:
    std::shared_ptr<SI::Frame> _selectedFrame;

    FramePropertiesEditor _frameParameterEditor;

    Gtk::Box _frameObjectBox;
    FrameObjectListEditor _frameObjectList;
    FrameObjectEditor _frameObjectEditor;

    Gtk::Box _actionPointBox;
    ActionPointListEditor _actionPointList;
    ActionPointEditor _actionPointEditor;

    Gtk::Box _entityHitboxBox;
    EntityHitboxListEditor _entityHitboxList;
    EntityHitboxEditor _entityHitboxEditor;

    enum {
        FRAME_PARAMETERS_PAGE,
        FRAME_OBJECT_PAGE,
        ACTION_POINT_PAGE,
        ENTITY_HITBOX_PAGE,
    };
};
}
}
}

#endif
