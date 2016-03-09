#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SPRITEIMPORTEREDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SPRITEIMPORTEREDITOR_H_

#include "frameeditor.h"
#include "framelist.h"
#include "framesetgraphicaleditor.h"
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

class SpriteImporterEditor {
public:
    SpriteImporterEditor();

    void setFrameSetList(SI::FrameSet::list_t* frameSetList);
    void setFrameSet(std::shared_ptr<SI::FrameSet> frameSet);
    void setFrame(std::shared_ptr<SI::Frame> frame);
    void setFrameObject(std::shared_ptr<SI::FrameObject> frameObject);
    void setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint);
    void setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox);

public:
    Gtk::Paned widget;

private:
    Gtk::ScrolledWindow _graphicalWindow;
    FrameSetGraphicalEditor _graphicalEditor;

    Gtk::Notebook _sidebar;
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
