#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIGNALS_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIGNALS_H_

#include "models/sprite-importer/frameobject.h"

#include <sigc++/signal.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

static sigc::signal<void, const std::shared_ptr<SI::FrameObject>> signal_frameObjectChanged;
static sigc::signal<void, const SI::FrameObject::list_t*> signal_frameObjectListChanged;
}
}
}

#endif
