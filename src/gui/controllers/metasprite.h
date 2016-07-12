#pragma once

#include "gui/controllers/basecontroller.h"
#include "gui/controllers/helpers/documentcontroller.h"
#include "gui/controllers/helpers/namedlistcontroller.h"
#include "gui/controllers/helpers/orderedlistcontroller.h"
#include "gui/controllers/helpers/spriteselectedtypecontroller.h"
#include "gui/controllers/metasprite-common.h"
#include "models/metasprite.h"

namespace UnTech {
namespace MetaSprite {

class MetaSpriteController;

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

    void selected_setLocation(const UnTech::ms8point& location);
    void selected_setTileId(const unsigned& tileId);
    void selected_setSize(const FrameObject::ObjectSize& size);
    void selected_setSizeAndTileId(const FrameObject::ObjectSize& size,
                                   const unsigned& tileId);
    void selected_setOrder(const unsigned& order);
    void selected_setHFlip(const bool& hFlip);
    void selected_setVFlip(const bool& vFlip);

    void selected_setLocation_merge(const UnTech::ms8point& location);
    void selected_setTileId_merge(const unsigned& tileId);

    auto& signal_sizeChanged() { return _signal_sizeChanged; }

private:
    sigc::signal<void, const FrameObject*> _signal_sizeChanged;
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

    void selected_setLocation(const UnTech::ms8point& location);
    void selected_setParameter(const unsigned& parameter);

    void selected_setLocation_merge(const UnTech::ms8point& location);
    void selected_setParameter_merge(const unsigned& parameter);
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

    void selected_setAabb(const UnTech::ms8rect& aabb);
    void selected_setParameter(const unsigned& parameter);

    void selected_setAabb_merge(const UnTech::ms8rect& aabb);
    void selected_setParameter_merge(const unsigned& parameter);
};

/*
 * FRAME CONTROLLER
 * ================
 */
class FrameController : public Controller::NamedListController<Frame> {
    friend class MetaSpriteController;

public:
    FrameController(Controller::BaseController& baseController)
        : NamedListController<Frame>(baseController)
    {
    }
    ~FrameController() = default;

    void selected_setSolid(const bool& solid);
    void selected_setTileHitbox(const UnTech::ms8rect& aabb);

    void selected_setTileHitbox_merge(const UnTech::ms8rect& aabb);
};

/*
 * PALETTE CONTROLLER
 * ==================
 */
class PaletteController : public Controller::OrderedListController<Palette> {
    friend class MetaSpriteController;

public:
    PaletteController(Controller::BaseController& baseController)
        : OrderedListController<Palette>(baseController)
        , _selectedColorId(-1)
    {
    }
    ~PaletteController() = default;

    // If < 0 then no color is selected.
    int selectedColorId() { return _selectedColorId; }
    void setSelectedColorId(unsigned colorId);
    void unsetSelectedColor();

    void selected_setColor(size_t index, const Snes::SnesColor& color);
    void selected_setColor_merge(size_t index, const Snes::SnesColor& color);

    auto& signal_selectedColorChanged() { return _signal_selectedColorChanged; }

private:
    int _selectedColorId;
    sigc::signal<void> _signal_selectedColorChanged;
};

/*
 * FRAME SET CONTROLLER
 * ====================
 */
class FrameSetController : public Controller::SingleItemController<FrameSet> {
    friend class MetaSpriteController;

public:
    FrameSetController(Controller::BaseController& baseController)
        : SingleItemController<FrameSet>(baseController)
    {
    }
    ~FrameSetController() = default;

    template <class TilesetT>
    void selected_tileset_setPixel(const unsigned tileId,
                                   unsigned x, unsigned y,
                                   unsigned value);

    void selected_smallTileset_setPixel(const unsigned tileId,
                                        unsigned x, unsigned y,
                                        unsigned value);

    void selected_largeTileset_setPixel(const unsigned tileId,
                                        unsigned x, unsigned y,
                                        unsigned value);

    void selected_addTiles(unsigned smallTiles, unsigned largeTiles);

    template <class TilesetT>
    sigc::signal<void, const FrameSet*>& signal_tilesetChanged();

    auto& signal_smallTilesetChanged() { return _signal_smallTilesetChanged; }
    auto& signal_largeTilesetChanged() { return _signal_largeTilesetChanged; }
    auto& signal_tileCountChanged() { return _signal_tileCountChanged; }

private:
    sigc::signal<void, const FrameSet*> _signal_smallTilesetChanged;
    sigc::signal<void, const FrameSet*> _signal_largeTilesetChanged;
    sigc::signal<void, const FrameSet*> _signal_tileCountChanged;
};

/*
 * META SPRITE CONTROLLER
 * ======================
 */
class MetaSpriteController : public Controller::DocumentController<MetaSpriteDocument> {
public:
    typedef UnTech::Controller::SpriteSelectedTypeController<MetaSpriteController> SelectedTypeController;

public:
    MetaSpriteController(std::unique_ptr<Controller::ControllerInterface>);
    MetaSpriteController(const MetaSpriteController&) = delete;
    ~MetaSpriteController() = default;

    auto& abstractFrameSetController() { return _abstractFrameSetController; }
    auto& frameSetController() { return _frameSetController; }
    auto& paletteController() { return _paletteController; }
    auto& frameController() { return _frameController; }
    auto& entityHitboxController() { return _entityHitboxController; }
    auto& actionPointController() { return _actionPointController; }
    auto& frameObjectController() { return _frameObjectController; }
    auto& selectedTypeController() { return _selectedTypeController; }

    // emits `signal_listChanged` and `signal_selectedChanged` for
    // all child controllers.
    void emitAllDataChanged();

private:
    FrameSetController _frameSetController;
    MetaSpriteCommon::AbstractFrameSetController _abstractFrameSetController;
    PaletteController _paletteController;
    FrameController _frameController;
    EntityHitboxController _entityHitboxController;
    ActionPointController _actionPointController;
    FrameObjectController _frameObjectController;
    SelectedTypeController _selectedTypeController;
};
}
}
