#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SPRITEIMPORTEREDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SPRITEIMPORTEREDITOR_H_

#include "actionpointeditor.h"
#include "actionpointlist.h"
#include "entityhitboxeditor.h"
#include "entityhitboxlist.h"
#include "framelist.h"
#include "frameobjecteditor.h"
#include "frameobjectlist.h"
#include "framepropertieseditor.h"
#include "framesetgraphicaleditor.h"
#include "framesetpropertieseditor.h"
#include "selection.h"
#include "models/sprite-importer.h"

#include <gtkmm.h>
#include <memory>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class SpriteImporterEditor {
public:
    SpriteImporterEditor();

    SI::SpriteImporterDocument* document() const { return _document.get(); }
    Selection& selection() { return _selection; }
    const Selection& selection() const { return _selection; }

    void setDocument(std::unique_ptr<SI::SpriteImporterDocument> document);

    void setZoom(int zoom, double aspectRatio);

public:
    Gtk::Paned widget;

private:
    std::unique_ptr<SI::SpriteImporterDocument> _document;

    Selection _selection;

    Gtk::ScrolledWindow _graphicalWindow;
    FrameSetGraphicalEditor _graphicalEditor;

    Gtk::Notebook _sidebar;

    Gtk::Paned _framePane;

    FrameSetPropertiesEditor _frameSetPropertiesEditor;

    FrameListEditor _frameList;

    Gtk::Notebook _frameNotebook;
    FramePropertiesEditor _frameParameterEditor;

    Gtk::Box _frameObjectBox;
    FrameObjectListEditor _frameObjectList;
    FrameObjectEditor _frameObjectEditor;

    Gtk::Box _actionPointBox;
    ActionPointListEditor _actionPointList;
    ActionPointEditor _actionPointEditor;

    Gtk::Box _entityHitboxBox;
    EntityHitboxListEditor _entityHitboxList;
    EntityHitboxEditor _entityHitboxEditor;

    enum FrameSetPages {
        FRAMESET_PAGE,
        FRAME_PAGE,
    };

    enum FramePages {
        FRAME_PARAMETERS_PAGE,
        FRAME_OBJECT_PAGE,
        ACTION_POINT_PAGE,
        ENTITY_HITBOX_PAGE,
    };
};
}
}
}

#endif
