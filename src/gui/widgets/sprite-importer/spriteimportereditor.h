#pragma once

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
#include "gui/controllers/sprite-importer.h"
#include "gui/widgets/metasprite-common/abstractframesetpropertieseditor.h"
#include "gui/widgets/metasprite-common/animationeditor.h"

#include <gtkmm.h>
#include <memory>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class SpriteImporterEditor {
public:
    SpriteImporterEditor(SI::SpriteImporterController& controller);

public:
    Gtk::Paned widget;

private:
    SI::SpriteImporterController& _controller;

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

    MetaSpriteCommon::AnimationEditor _animationEditor;

    enum SidebarPages {
        FRAMESET_PAGE,
        FRAME_PAGE,
        ANIMATION_PAGE,
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
