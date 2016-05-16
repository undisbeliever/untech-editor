#include "signals.h"

namespace UnTech {
namespace Widgets {
namespace MetaSpriteFormat {
namespace Signals {

namespace MSF = UnTech::MetaSpriteFormat;

sigc::signal<void, const MSF::AbstractFrameSet*> abstractFrameSetChanged;
sigc::signal<void, const MSF::AbstractFrameSet*> abstractFrameSetNameChanged;
sigc::signal<void, const MSF::AbstractFrameSet*> abstractFrameSetExportOrderChanged;
}
}
}
}
