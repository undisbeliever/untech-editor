#pragma once

#include "gui/controllers/basecontroller.h"
#include "gui/controllers/helpers/documentcontroller.h"
#include "gui/controllers/helpers/namedlistcontroller.h"
#include "gui/controllers/helpers/orderedlistcontroller.h"
#include "gui/controllers/helpers/spriteselectedtypecontroller.h"
#include "gui/controllers/metasprite-format.h"
#include "models/sprite-importer.h"

namespace UnTech {
namespace SpriteImporter {

class SpriteImporterController;

/*
 * FRAME OBJECT CONTROLLER
 * =======================
 */
class FrameObjectController : public Controller::OrderedListController<FrameObject> {
public:
    FrameObjectController(Controller::BaseController& baseController)
        : OrderedListController<FrameObject>(baseController)
    {
    }
    ~FrameObjectController() = default;

    void selected_setLocation(const UnTech::upoint& location);

    // settings size may change location, so we save both.
    void selected_setLocationAndSize(const UnTech::upoint& location,
                                     const FrameObject::ObjectSize& size);

    void selected_setLocation_merge(const UnTech::upoint& location);
};

/*
 * ACTION POINT CONTROLLER
 * =======================
 */
class ActionPointController : public Controller::OrderedListController<ActionPoint> {
public:
    ActionPointController(Controller::BaseController& baseController)
        : OrderedListController<ActionPoint>(baseController)
    {
    }
    ~ActionPointController() = default;

    void selected_setLocation(const UnTech::upoint& location);
    void selected_setParameter(const unsigned& tileId);

    void selected_setLocation_merge(const UnTech::upoint& location);
};

/*
 * ENTITY HITBOX CONTROLLER
 * ========================
 */
class EntityHitboxController : public Controller::OrderedListController<EntityHitbox> {
public:
    EntityHitboxController(Controller::BaseController& baseController)
        : OrderedListController<EntityHitbox>(baseController)
    {
    }
    ~EntityHitboxController() = default;

    void selected_setAabb(const UnTech::urect& aabb);
    void selected_setParameter(const unsigned& tileId);

    void selected_setAabb_merge(const UnTech::urect& aabb);
};

/*
 * FRAME CONTROLLER
 * ================
 */
class FrameController : public Controller::NamedListController<Frame> {
    friend class SpriteImporterController;

public:
    FrameController(Controller::BaseController& baseController)
        : NamedListController<Frame>(baseController)
    {
    }
    ~FrameController() = default;

    void selected_setLocation(const UnTech::urect& location);
    void selected_setGridLocation(const UnTech::upoint& gridLocation);
    void selected_setUseGridLocation(const bool& useGridLocation);

    void selected_setOrigin(const UnTech::upoint& origin);
    void selected_setUseGridOrigin(const bool& useGridOrigin);

    void selected_setSolid(const bool& solid);
    void selected_setTileHitbox(const UnTech::urect& aabb);

    void selected_setSpriteOrder(const unsigned& order);

    void selected_setGridLocation_merge(const UnTech::upoint& gridLocation);
    void selected_setOrigin_merge(const UnTech::upoint& origin);
    void selected_setLocation_merge(const UnTech::urect& location);
    void selected_setTileHitbox_merge(const UnTech::urect& aabb);

    auto& signal_frameSizeChanged() { return _signal_frameSizeChanged; }

public:
    sigc::signal<void, const Frame*> _signal_frameSizeChanged;
};

/*
 * FRAME SET CONTROLLER
 * ====================
 */
class FrameSetController : public Controller::SingleItemController<FrameSet> {
    friend class SpriteImporterController;

public:
    FrameSetController(Controller::BaseController& baseController)
        : SingleItemController<FrameSet>(baseController)
    {
    }
    ~FrameSetController() = default;

    bool selectTransparentMode() const { return _selectTransparentMode; }
    void setSelectTransparentMode(bool mode);

    void selected_setTransparentColor(const UnTech::rgba& color);
    void selected_setImageFilename(const std::string& filename);

    void selected_setGridFrameSize(const UnTech::usize& size);
    void selected_setGridOffset(const UnTech::upoint& offset);
    void selected_setGridPadding(const UnTech::usize& passing);
    void selected_setGridOrigin(const UnTech::upoint& origin);

    void selected_setGridFrameSize_merge(const UnTech::usize& size);
    void selected_setGridOffset_merge(const UnTech::upoint& offset);
    void selected_setGridPadding_merge(const UnTech::usize& passing);
    void selected_setGridOrigin_merge(const UnTech::upoint& origin);

    auto& signal_imageChanged() { return _signal_imageChanged; }
    auto& signal_gridChanged() { return _signal_gridChanged; }

    auto& signal_selectTransparentModeChanged() { return _signal_selectTransparentModeChanged; }

private:
    bool _selectTransparentMode = false;
    sigc::signal<void, const FrameSet*> _signal_imageChanged;
    sigc::signal<void, const FrameSet*> _signal_gridChanged;
    sigc::signal<void> _signal_selectTransparentModeChanged;
};

/*
 * SPRITE IMPORTER CONTROLLER
 * ==========================
 */
class SpriteImporterController : public Controller::DocumentController<SpriteImporterDocument> {
public:
    typedef UnTech::Controller::SpriteSelectedTypeController<SpriteImporterController> SelectedTypeController;

public:
    SpriteImporterController(std::unique_ptr<Controller::ControllerInterface>);
    SpriteImporterController(const SpriteImporterController&) = delete;
    ~SpriteImporterController() = default;

    auto& abstractFrameSetController() { return _abstractFrameSetController; }
    auto& frameSetController() { return _frameSetController; }
    auto& frameController() { return _frameController; }
    auto& entityHitboxController() { return _entityHitboxController; }
    auto& actionPointController() { return _actionPointController; }
    auto& frameObjectController() { return _frameObjectController; }
    auto& selectedTypeController() { return _selectedTypeController; }

private:
    FrameSetController _frameSetController;
    MetaSpriteFormat::AbstractFrameSetController _abstractFrameSetController;
    FrameController _frameController;
    EntityHitboxController _entityHitboxController;
    ActionPointController _actionPointController;
    FrameObjectController _frameObjectController;
    SelectedTypeController _selectedTypeController;
};
}
}
