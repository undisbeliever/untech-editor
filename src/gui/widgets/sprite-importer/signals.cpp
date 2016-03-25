#include "signals.h"

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {
namespace Signals {

namespace SI = UnTech::SpriteImporter;

sigc::signal<void, const std::shared_ptr<SI::FrameSet>> frameSetChanged;
sigc::signal<void, const SI::FrameSet::list_t*> frameSetListChanged;

sigc::signal<void, const std::shared_ptr<SI::FrameSet>> frameSetGridChanged;

sigc::signal<void, const std::shared_ptr<SI::Frame>> frameChanged;
sigc::signal<void, const std::shared_ptr<SI::Frame>> frameSizeChanged;
sigc::signal<void, const SI::Frame::list_t*> frameListChanged;

sigc::signal<void, const std::shared_ptr<SI::FrameObject>> frameObjectChanged;
sigc::signal<void, const SI::FrameObject::list_t*> frameObjectListChanged;

sigc::signal<void, const std::shared_ptr<SI::ActionPoint>> actionPointChanged;
sigc::signal<void, const SI::ActionPoint::list_t*> actionPointListChanged;

sigc::signal<void, const std::shared_ptr<SI::EntityHitbox>> entityHitboxChanged;
sigc::signal<void, const SI::EntityHitbox::list_t*> entityHitboxListChanged;
}
}
}
}
