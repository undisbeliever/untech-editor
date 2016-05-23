#pragma once

#include "models/metasprite-format/abstractframeset.h"

#include <sigc++/signal.h>

namespace UnTech {
namespace Widgets {
namespace MetaSpriteFormat {

/**
 * Global signals that are used to notify the GUI when the data has
 * changed.
 *
 * This is done globally so that windows/tabs that require the same data
 * are updated. The overhead of doing things this way is minimal.
 */
namespace Signals {

namespace MSF = UnTech::MetaSpriteFormat;

extern sigc::signal<void, const MSF::AbstractFrameSet*> abstractFrameSetChanged;
extern sigc::signal<void, const MSF::AbstractFrameSet*> abstractFrameSetNameChanged;
extern sigc::signal<void, const MSF::AbstractFrameSet*> abstractFrameSetExportOrderChanged;
}
}
}
}
