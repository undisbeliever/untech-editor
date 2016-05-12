#include "signals.h"

namespace UnTech {
namespace Widgets {
namespace MetaSprite {
namespace Signals {

namespace MS = UnTech::MetaSprite;

sigc::signal<void, const MS::FrameSet*> frameSetChanged;
sigc::signal<void, const MS::FrameSet*> frameSetTilesetChanged;
sigc::signal<void, const MS::FrameSet*> frameSetTilesetCountChanged;
sigc::signal<void, const MS::FrameSet*> frameSetPaletteChanged;
sigc::signal<void, const MS::FrameSet*> frameSetExportOrderChanged;

sigc::signal<void, const MS::Palette*> paletteChanged;
sigc::signal<void, const MS::Palette::list_t*> paletteListChanged;

sigc::signal<void, const MS::Frame*> frameChanged;
sigc::signal<void, const MS::Frame*> frameSizeChanged;
sigc::signal<void, const MS::Frame::list_t*> frameListChanged;

sigc::signal<void, const MS::FrameObject*> frameObjectChanged;
sigc::signal<void, const MS::FrameObject::list_t*> frameObjectListChanged;

sigc::signal<void, const MS::ActionPoint*> actionPointChanged;
sigc::signal<void, const MS::ActionPoint::list_t*> actionPointListChanged;

sigc::signal<void, const MS::EntityHitbox*> entityHitboxChanged;
sigc::signal<void, const MS::EntityHitbox::list_t*> entityHitboxListChanged;
}
}
}
}
