#include "signals.h"

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {
namespace Signals {

namespace SI = UnTech::SpriteImporter;

sigc::signal<void, const SI::FrameSet*> frameSetChanged;
sigc::signal<void, const SI::FrameSet*> frameSetImageChanged;
sigc::signal<void, const SI::FrameSet*> frameSetGridChanged;
sigc::signal<void, const SI::FrameSet*> frameSetExportOrderChanged;

sigc::signal<void, const SI::Frame*> frameChanged;
sigc::signal<void, const SI::Frame*> frameSizeChanged;
sigc::signal<void, const SI::Frame::list_t*> frameListChanged;

sigc::signal<void, const SI::FrameObject*> frameObjectChanged;
sigc::signal<void, const SI::FrameObject::list_t*> frameObjectListChanged;

sigc::signal<void, const SI::ActionPoint*> actionPointChanged;
sigc::signal<void, const SI::ActionPoint::list_t*> actionPointListChanged;

sigc::signal<void, const SI::EntityHitbox*> entityHitboxChanged;
sigc::signal<void, const SI::EntityHitbox::list_t*> entityHitboxListChanged;
}
}
}
}
