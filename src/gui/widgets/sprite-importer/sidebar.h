#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIDEBAR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIDEBAR_H_

#include "frameeditor.h"
#include "framelist.h"
#include "framesetlist.h"
#include "framesetpropertieseditor.h"
#include "models/sprite-importer.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class Sidebar {
public:
    Sidebar();

    void setFrameSetList(SI::FrameSet::list_t* frameSetList);
    void setFrameSet(std::shared_ptr<SI::FrameSet> frameSet);
    void setFrame(std::shared_ptr<SI::Frame> frame);
    void setFrameObject(std::shared_ptr<SI::FrameObject> frameObject);
    void setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint);
    void setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox);

public:
    Gtk::Notebook widget;

private:
    std::shared_ptr<SI::FrameSet> _selectedFrameSet;
    std::shared_ptr<SI::Frame> _selectedFrame;

    Gtk::Paned _frameSetPane, _framePane;

    FrameSetListEditor _frameSetList;
    FrameSetPropertiesEditor _frameSetPropertiesEditor;

    FrameListEditor _frameList;
    FrameEditor _frameEditor;

    enum {
        FRAMESET_PAGE,
        FRAME_PAGE
    };
};
}
}
}

#endif
