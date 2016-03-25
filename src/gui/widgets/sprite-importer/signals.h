#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIGNALS_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIGNALS_H_

#include "models/sprite-importer.h"

#include <sigc++/signal.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

/**
 * Global signals that are used to notify the GUI when the data has
 * changed.
 *
 * This is done globally so that windows/tabs that require the same data
 * are updated. The overhead of doing things this way is minimal.
 */
namespace Signals {

namespace SI = UnTech::SpriteImporter;

extern sigc::signal<void, const std::shared_ptr<SI::FrameSet>> frameSetChanged;
extern sigc::signal<void, const SI::FrameSet::list_t*> frameSetListChanged;

extern sigc::signal<void, const std::shared_ptr<SI::FrameSet>> frameSetGridChanged;

extern sigc::signal<void, const std::shared_ptr<SI::Frame>> frameChanged;
extern sigc::signal<void, const std::shared_ptr<SI::Frame>> frameSizeChanged;
extern sigc::signal<void, const SI::Frame::list_t*> frameListChanged;

extern sigc::signal<void, const std::shared_ptr<SI::FrameObject>> frameObjectChanged;
extern sigc::signal<void, const SI::FrameObject::list_t*> frameObjectListChanged;

extern sigc::signal<void, const std::shared_ptr<SI::ActionPoint>> actionPointChanged;
extern sigc::signal<void, const SI::ActionPoint::list_t*> actionPointListChanged;

extern sigc::signal<void, const std::shared_ptr<SI::EntityHitbox>> entityHitboxChanged;
extern sigc::signal<void, const SI::EntityHitbox::list_t*> entityHitboxListChanged;
}
}
}
}

#endif
