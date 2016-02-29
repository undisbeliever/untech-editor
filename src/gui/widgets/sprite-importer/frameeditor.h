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
    FrameEditor();

    void setFrame(std::shared_ptr<SI::Frame> frame);
    void setFrameObject(std::shared_ptr<SI::FrameObject> frameObject);
    void setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint);
    void setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox);

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
