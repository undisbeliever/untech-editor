#include "signals.h"

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

sigc::signal<void, const std::shared_ptr<SI::FrameSet>> signal_frameSetChanged;
sigc::signal<void, const SI::FrameSet::list_t*> signal_frameSetListChanged;

sigc::signal<void, const std::shared_ptr<SI::FrameSet>> signal_frameSetGridChanged;

sigc::signal<void, const std::shared_ptr<SI::Frame>> signal_frameChanged;
sigc::signal<void, const std::shared_ptr<SI::Frame>> signal_frameSizeChanged;
sigc::signal<void, const SI::Frame::list_t*> signal_frameListChanged;

sigc::signal<void, const std::shared_ptr<SI::FrameObject>> signal_frameObjectChanged;
sigc::signal<void, const SI::FrameObject::list_t*> signal_frameObjectListChanged;

sigc::signal<void, const std::shared_ptr<SI::ActionPoint>> signal_actionPointChanged;
sigc::signal<void, const std::shared_ptr<SI::ActionPoint>> signal_actionPointLocationChanged;
sigc::signal<void, const SI::ActionPoint::list_t*> signal_actionPointListChanged;

sigc::signal<void, const std::shared_ptr<SI::EntityHitbox>> signal_entityHitboxChanged;
sigc::signal<void, const std::shared_ptr<SI::EntityHitbox>> signal_entityHitboxLocationChanged;
sigc::signal<void, const SI::EntityHitbox::list_t*> signal_entityHitboxListChanged;
}
}
}
