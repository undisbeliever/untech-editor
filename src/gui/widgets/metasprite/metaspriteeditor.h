#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_METASPRITEEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_METASPRITEEDITOR_H_

#include "actionpointeditor.h"
#include "actionpointlist.h"
#include "entityhitboxeditor.h"
#include "entityhitboxlist.h"
#include "framelist.h"
#include "frameobjecteditor.h"
#include "frameobjectlist.h"
#include "framegraphicaleditor.h"
#include "framepropertieseditor.h"
#include "paletteeditor.h"
#include "palettelist.h"
#include "selection.h"
#include "tileseteditor.h"
#include "models/metasprite.h"
#include "gui/widgets/metasprite-format/abstractframesetpropertieseditor.h"

#include <memory>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class MetaSpriteEditor {
public:
    MetaSpriteEditor();

    MS::MetaSpriteDocument* document() const { return _document.get(); }
    Selection& selection() { return _selection; }
    const Selection& selection() const { return _selection; }

    void setDocument(std::unique_ptr<MS::MetaSpriteDocument> document);

    void setShowTwoEditors(bool showTwoEditors);
    void setZoom(int zoom, double aspectRatio);

protected:
    void on_scroll_changed();

public:
    Gtk::Paned widget;

private:
    std::unique_ptr<MS::MetaSpriteDocument> _document;

    Selection _selection;

    Gtk::Box _rightSideBox;

    unsigned _selectedGraphicalEditor;
    FrameGraphicalEditor _graphicalEditor0, _graphicalEditor1;
    Gtk::Paned _graphicalContainer;
    Gtk::Scrollbar _graphicalHScroll, _graphicalVScroll;
    Gtk::Grid _graphicalGrid;

    TilesetEditor _tilesetEditor;

    Gtk::Notebook _sidebar;

    Gtk::Paned _framePane;

    Gtk::Box _frameSetBox;
    MetaSpriteFormat::AbstractFrameSetPropertiesEditor _frameSetPropertiesEditor;

    Gtk::Frame _paletteFrame;
    Gtk::Box _paletteBox;
    PaletteListEditor _paletteList;
    PaletteEditor _paletteEditor;

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
