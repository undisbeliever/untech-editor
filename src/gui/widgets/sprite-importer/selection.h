#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SELECTION_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SELECTION_H_

#include "models/sprite-importer.h"

#include <glibmm/ustring.h>
#include <sigc++/signal.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class Selection {
public:
    enum class Type {
        NONE = 0,
        FRAME_OBJECT,
        ACTION_POINT,
        ENTITY_HITBOX
    };

    void setFrameSet(SI::FrameSet* frameSet);
    void setFrame(SI::Frame* frame);
    void setFrameObject(SI::FrameObject* frameObject);
    void setActionPoint(SI::ActionPoint* actionPoint);
    void setEntityHitbox(SI::EntityHitbox* entityHitbox);

    void setSelectTransparentMode(bool v);

    // WILL ALWAYS emit a selectionChanged
    void unselectAll();

    Glib::ustring typeString() const;

    Type type() const { return _type; }
    SI::FrameSet* frameSet() const { return _frameSet; }
    SI::Frame* frame() const { return _frame; }
    SI::FrameObject* frameObject() const { return _frameObject; }
    SI::ActionPoint* actionPoint() const { return _actionPoint; }
    SI::EntityHitbox* entityHitbox() const { return _entityHitbox; }

    bool selectTransparentMode() const { return _selectTransparentMode; }

    bool canCrudSelected() const { return _type != Type::NONE; }
    bool canMoveSelectedUp() const;
    bool canMoveSelectedDown() const;

    void createNewOfSelectedType();
    void cloneSelected();
    void removeSelected();
    void moveSelectedUp();
    void moveSelectedDown();

    sigc::signal<void> signal_selectionChanged;
    sigc::signal<void> signal_frameSetChanged;
    sigc::signal<void> signal_frameChanged;
    sigc::signal<void> signal_frameObjectChanged;
    sigc::signal<void> signal_actionPointChanged;
    sigc::signal<void> signal_entityHitboxChanged;
    sigc::signal<void> signal_selectTransparentModeChanged;

private:
    // return true if the frame changed
    // Does not unselect frameObject, actionPoint, entityHitbox
    // caller has to do that.
    bool updateFrameIfDifferent(SI::Frame& frame);

private:
    Type _type = Type::NONE;
    SI::FrameSet* _frameSet = nullptr;
    SI::Frame* _frame = nullptr;
    SI::FrameObject* _frameObject = nullptr;
    SI::ActionPoint* _actionPoint = nullptr;
    SI::EntityHitbox* _entityHitbox = nullptr;

    bool _selectTransparentMode = false;
};
}
}
}
#endif
