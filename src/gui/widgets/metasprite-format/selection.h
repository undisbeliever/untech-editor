#ifndef _UNTECH_GUI_WIDGETS_METASPRITEFORMAT_SELECTION_H_
#define _UNTECH_GUI_WIDGETS_METASPRITEFORMAT_SELECTION_H_

#include "models/metasprite-format/abstractframeset.h"

#include <sigc++/signal.h>

namespace UnTech {
namespace Widgets {
namespace MetaSpriteFormat {

namespace MSF = UnTech::MetaSpriteFormat;

class Selection {
public:
    MSF::AbstractFrameSet* abstractFrameSet() const { return _abstractFrameSet; }

    // must be called when frameset of parent class changed.
    void setAbstractFrameSet(MSF::AbstractFrameSet* abstractFrameSet)
    {
        if (_abstractFrameSet != abstractFrameSet) {
            _abstractFrameSet = abstractFrameSet;
            signal_frameSetChanged.emit();
        }
    }

public:
    sigc::signal<void> signal_frameSetChanged;

private:
    MSF::AbstractFrameSet* _abstractFrameSet = nullptr;
};
}
}
}
#endif
