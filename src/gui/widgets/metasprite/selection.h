#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_SELECTION_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_SELECTION_H_

#include "models/metasprite.h"

#include <glibmm/ustring.h>
#include <sigc++/signal.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class Selection {
public:
    enum class Type {
        NONE = 0,
        FRAME_OBJECT,
        ACTION_POINT,
        ENTITY_HITBOX
    };

    void setFrameSet(std::shared_ptr<MS::FrameSet> frameSet);
    void setPalette(std::shared_ptr<MS::Palette> palette);
    void setFrame(std::shared_ptr<MS::Frame> frame);
    void setFrameObject(std::shared_ptr<MS::FrameObject> frameObject);
    void setActionPoint(std::shared_ptr<MS::ActionPoint> actionPoint);
    void setEntityHitbox(std::shared_ptr<MS::EntityHitbox> entityHitbox);

    // WILL ALWAYS emit a selectionChanged
    void unselectAll();

    Glib::ustring typeString() const;

    Type type() const { return _type; }
    std::shared_ptr<MS::FrameSet> frameSet() const { return _frameSet; }
    std::shared_ptr<MS::Palette> palette() const { return _palette; }
    std::shared_ptr<MS::Frame> frame() const { return _frame; }
    std::shared_ptr<MS::FrameObject> frameObject() const { return _frameObject; }
    std::shared_ptr<MS::ActionPoint> actionPoint() const { return _actionPoint; }
    std::shared_ptr<MS::EntityHitbox> entityHitbox() const { return _entityHitbox; }

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
    sigc::signal<void> signal_paletteChanged;
    sigc::signal<void> signal_frameChanged;
    sigc::signal<void> signal_frameObjectChanged;
    sigc::signal<void> signal_actionPointChanged;
    sigc::signal<void> signal_entityHitboxChanged;

private:
    // updates current without nulling child selection
    void updateFrameSet(std::shared_ptr<MS::FrameSet> frameSet);
    void updateFrame(std::shared_ptr<MS::Frame> frame);

    Type _type = Type::NONE;
    std::shared_ptr<MS::FrameSet> _frameSet = nullptr;
    std::shared_ptr<MS::Palette> _palette = nullptr;
    std::shared_ptr<MS::Frame> _frame = nullptr;
    std::shared_ptr<MS::FrameObject> _frameObject = nullptr;
    std::shared_ptr<MS::ActionPoint> _actionPoint = nullptr;
    std::shared_ptr<MS::EntityHitbox> _entityHitbox = nullptr;
};
}
}
}
#endif
