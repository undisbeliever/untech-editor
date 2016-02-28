#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIGNALS_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIGNALS_H_

#include "models/sprite-importer/frameobject.h"

#include <sigc++/signal.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

static sigc::signal<void, const std::shared_ptr<SI::FrameSet>> signal_frameSetChanged;
static sigc::signal<void, const SI::FrameSet::list_t*> signal_frameSetListChanged;

static sigc::signal<void, const std::shared_ptr<SI::FrameSet>> signal_frameSetGridChanged;

static sigc::signal<void, const std::shared_ptr<SI::Frame>> signal_frameChanged;
static sigc::signal<void, const std::shared_ptr<SI::Frame>> signal_frameSizeChanged;
static sigc::signal<void, const SI::Frame::list_t*> signal_frameListChanged;

static sigc::signal<void, const std::shared_ptr<SI::FrameObject>> signal_frameObjectChanged;
static sigc::signal<void, const SI::FrameObject::list_t*> signal_frameObjectListChanged;

static sigc::signal<void, const std::shared_ptr<SI::ActionPoint>> signal_actionPointChanged;
static sigc::signal<void, const std::shared_ptr<SI::ActionPoint>> signal_actionPointLocationChanged;
static sigc::signal<void, const SI::ActionPoint::list_t*> signal_actionPointListChanged;

static sigc::signal<void, const std::shared_ptr<SI::EntityHitbox>> signal_entityHitboxChanged;
static sigc::signal<void, const std::shared_ptr<SI::EntityHitbox>> signal_entityHitboxLocationChanged;
static sigc::signal<void, const SI::EntityHitbox::list_t*> signal_entityHitboxListChanged;
}
}
}

#endif
