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

    void setFrameSet(std::shared_ptr<SI::FrameSet> frameSet);
    void setFrame(std::shared_ptr<SI::Frame> frame);
    void setFrameObject(std::shared_ptr<SI::FrameObject> frameObject);
    void setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint);
    void setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox);

    // WILL ALWAYS emit a selectionChanged
    void unselectAll();

    Glib::ustring typeString() const;

    Type type() const { return _type; }
    std::shared_ptr<SI::FrameSet> frameSet() const { return _frameSet; }
    std::shared_ptr<SI::Frame> frame() const { return _frame; }
    std::shared_ptr<SI::FrameObject> frameObject() const { return _frameObject; }
    std::shared_ptr<SI::ActionPoint> actionPoint() const { return _actionPoint; }
    std::shared_ptr<SI::EntityHitbox> entityHitbox() const { return _entityHitbox; }

    sigc::signal<void> signal_selectionChanged;
    sigc::signal<void> signal_frameSetChanged;
    sigc::signal<void> signal_frameChanged;
    sigc::signal<void> signal_frameObjectChanged;
    sigc::signal<void> signal_actionPointChanged;
    sigc::signal<void> signal_entityHitboxChanged;

private:
    Type _type = Type::NONE;
    std::shared_ptr<SI::FrameSet> _frameSet = nullptr;
    std::shared_ptr<SI::Frame> _frame = nullptr;
    std::shared_ptr<SI::FrameObject> _frameObject = nullptr;
    std::shared_ptr<SI::ActionPoint> _actionPoint = nullptr;
    std::shared_ptr<SI::EntityHitbox> _entityHitbox = nullptr;
};
}
}
}
#endif
