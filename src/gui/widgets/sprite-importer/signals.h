#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIGNALS_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIGNALS_H_

#include "models/sprite-importer.h"

#include <sigc++/signal.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

extern sigc::signal<void, const std::shared_ptr<SI::FrameSet>> signal_frameSetChanged;
extern sigc::signal<void, const SI::FrameSet::list_t*> signal_frameSetListChanged;

extern sigc::signal<void, const std::shared_ptr<SI::FrameSet>> signal_frameSetGridChanged;

extern sigc::signal<void, const std::shared_ptr<SI::Frame>> signal_frameChanged;
extern sigc::signal<void, const std::shared_ptr<SI::Frame>> signal_frameSizeChanged;
extern sigc::signal<void, const SI::Frame::list_t*> signal_frameListChanged;

extern sigc::signal<void, const std::shared_ptr<SI::FrameObject>> signal_frameObjectChanged;
extern sigc::signal<void, const SI::FrameObject::list_t*> signal_frameObjectListChanged;

extern sigc::signal<void, const std::shared_ptr<SI::ActionPoint>> signal_actionPointChanged;
extern sigc::signal<void, const std::shared_ptr<SI::ActionPoint>> signal_actionPointLocationChanged;
extern sigc::signal<void, const SI::ActionPoint::list_t*> signal_actionPointListChanged;

extern sigc::signal<void, const std::shared_ptr<SI::EntityHitbox>> signal_entityHitboxChanged;
extern sigc::signal<void, const std::shared_ptr<SI::EntityHitbox>> signal_entityHitboxLocationChanged;
extern sigc::signal<void, const SI::EntityHitbox::list_t*> signal_entityHitboxListChanged;
}
}
}

#endif
