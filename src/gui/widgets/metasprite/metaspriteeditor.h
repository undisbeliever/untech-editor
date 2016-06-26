#pragma once

#include "actionpointeditor.h"
#include "actionpointlist.h"
#include "entityhitboxeditor.h"
#include "entityhitboxlist.h"
#include "framegraphicaleditor.h"
#include "framelist.h"
#include "frameobjecteditor.h"
#include "frameobjectlist.h"
#include "framepropertieseditor.h"
#include "paletteeditor.h"
#include "palettelist.h"
#include "tileseteditor.h"
#include "gui/controllers/metasprite.h"
#include "gui/widgets/metasprite-common/abstractframesetpropertieseditor.h"

#include <gtkmm.h>
#include <memory>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class MetaSpriteEditor {
public:
    MetaSpriteEditor(MS::MetaSpriteController& controller);

    void setShowTwoEditors(bool showTwoEditors);

protected:
    void on_scroll_changed();

public:
    Gtk::Paned widget;

private:
    MS::MetaSpriteController& _controller;

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
    MetaSpriteCommon::AbstractFrameSetPropertiesEditor _frameSetPropertiesEditor;

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

    enum SidebarPages {
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
