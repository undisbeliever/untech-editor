#include "signals.h"

namespace UnTech {
namespace Widgets {
namespace MetaSprite {
namespace Signals {

namespace MS = UnTech::MetaSprite;

sigc::signal<void, const std::shared_ptr<MS::FrameSet>> frameSetChanged;
sigc::signal<void, const std::shared_ptr<MS::FrameSet>> frameSetTilesetChanged;
sigc::signal<void, const std::shared_ptr<MS::FrameSet>> frameSetTilesetCountChanged;
sigc::signal<void, const std::shared_ptr<MS::FrameSet>> frameSetPaletteChanged;

sigc::signal<void, const std::shared_ptr<MS::Frame>> frameChanged;
sigc::signal<void, const std::shared_ptr<MS::Frame>> frameSizeChanged;
sigc::signal<void, const MS::Frame::list_t*> frameListChanged;

sigc::signal<void, const std::shared_ptr<MS::FrameObject>> frameObjectChanged;
sigc::signal<void, const MS::FrameObject::list_t*> frameObjectListChanged;

sigc::signal<void, const std::shared_ptr<MS::ActionPoint>> actionPointChanged;
sigc::signal<void, const MS::ActionPoint::list_t*> actionPointListChanged;

sigc::signal<void, const std::shared_ptr<MS::EntityHitbox>> entityHitboxChanged;
sigc::signal<void, const MS::EntityHitbox::list_t*> entityHitboxListChanged;
}
}
}
}
