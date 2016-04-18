#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_MSGNALS_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_MSGNALS_H_

#include "models/metasprite.h"

#include <sigc++/signal.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

/**
 * Global signals that are used to notify the GUI when the data has
 * changed.
 *
 * This is done globally so that windows/tabs that require the same data
 * are updated. The overhead of doing things this way is minimal.
 */
namespace Signals {

namespace MS = UnTech::MetaSprite;

extern sigc::signal<void, const std::shared_ptr<MS::FrameSet>> frameSetChanged;
extern sigc::signal<void, const std::shared_ptr<MS::FrameSet>> frameSetTilesetChanged;
extern sigc::signal<void, const std::shared_ptr<MS::FrameSet>> frameSetTilesetCountChanged;
extern sigc::signal<void, const std::shared_ptr<MS::FrameSet>> frameSetPaletteChanged;

extern sigc::signal<void, const std::shared_ptr<MS::Palette>> paletteChanged;
extern sigc::signal<void, const MS::Palette::list_t*> paletteListChanged;

extern sigc::signal<void, const std::shared_ptr<MS::Frame>> frameChanged;
extern sigc::signal<void, const std::shared_ptr<MS::Frame>> frameSizeChanged;
extern sigc::signal<void, const MS::Frame::list_t*> frameListChanged;

extern sigc::signal<void, const std::shared_ptr<MS::FrameObject>> frameObjectChanged;
extern sigc::signal<void, const MS::FrameObject::list_t*> frameObjectListChanged;

extern sigc::signal<void, const std::shared_ptr<MS::ActionPoint>> actionPointChanged;
extern sigc::signal<void, const MS::ActionPoint::list_t*> actionPointListChanged;

extern sigc::signal<void, const std::shared_ptr<MS::EntityHitbox>> entityHitboxChanged;
extern sigc::signal<void, const MS::EntityHitbox::list_t*> entityHitboxListChanged;
}
}
}
}

#endif
