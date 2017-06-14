/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animation.h"
#include "selected.h"
#include "settings.h"
#include "gui-wx/controllers/basecontroller.h"
#include "gui-wx/controllers/containers/cappedvectorcontroller.h"
#include "gui-wx/controllers/containers/idmapcontroller.h"
#include "gui-wx/controllers/containers/sharedptrrootcontroller.h"
#include "models/metasprite/metasprite.h"

namespace UnTech {
namespace MetaSprite {
namespace MetaSprite {

class MetaSpriteController;
class PaletteController;
class FrameController;
class FrameObjectController;
class ActionPointController;
class EntityHitboxController;

class FrameSetController
    : public Controller::SharedPtrRootController<FrameSet> {

    friend class MetaSpriteController;
    friend class Animation::AnimationControllerImpl<FrameSetController>;

public:
    FrameSetController(Controller::BaseController& baseController)
        : SharedPtrRootController(baseController)
    {
    }

    void selected_setName(const idstring& name);
    void selected_setTilesetType(const TilesetType tilesetType);
    void selected_setExportOrderFilename(const std::string& filename);

    void selected_addTiles(unsigned smallTiles, unsigned largeTiles);

    void selected_smallTileset_setPixel(
        unsigned tileId, unsigned x, unsigned y, unsigned value);

    void selected_largeTileset_setPixel(
        unsigned tileId, unsigned x, unsigned y, unsigned value);
};

class PaletteController
    : public Controller::CappedVectorController<Snes::Palette4bpp,
                                                capped_vector<Snes::Palette4bpp, MAX_PALETTES>,
                                                FrameSetController> {
public:
    PaletteController(FrameSetController& parent);

    // If < 0 then no color is selected.
    int selectedColorId() { return _selectedColorId; }
    void setSelectedColorId(unsigned colorId);
    void unsetSelectedColor();

    void selected_setColor(size_t index, const Snes::SnesColor& color);

    auto& signal_selectedColorChanged() { return _signal_selectedColorChanged; }

private:
    int _selectedColorId;
    sigc::signal<void> _signal_selectedColorChanged;
};

class FrameController
    : public Controller::IdMapController<Frame, FrameSetController> {

public:
    FrameController(FrameSetController& parent)
        : IdMapController(parent)
    {
    }

    void selected_setSpriteOrder(const SpriteOrderType& spriteOrder);
    void selected_setSolid(const bool solid);
    void selected_setTileHitbox(const ms8rect& tileHitbox);
};

class FrameObjectController
    : public Controller::CappedVectorController<FrameObject, FrameObject::list_t,
                                                FrameController> {

public:
    FrameObjectController(FrameController& parent)
        : CappedVectorController(parent)
    {
    }

    virtual void onCreate(FrameObject& obj) final;

    void selected_setLocation(const ms8point& location);
    void selected_setTileId(const unsigned tileId);
    void selected_setSize(const ObjectSize size);
    void selected_setSizeAndTileId(const ObjectSize size, const unsigned tileId);
    void selected_setHFlip(const bool hFlip);
    void selected_setVFlip(const bool vFlip);
};

class ActionPointController
    : public Controller::CappedVectorController<ActionPoint, ActionPoint::list_t,
                                                FrameController> {

public:
    ActionPointController(FrameController& parent)
        : CappedVectorController(parent)
    {
    }

    void selected_setLocation(const ms8point& location);
    void selected_setParameter(const ActionPointParameter& parameter);
};

class EntityHitboxController
    : public Controller::CappedVectorController<EntityHitbox, EntityHitbox::list_t,
                                                FrameController> {

public:
    EntityHitboxController(FrameController& parent)
        : CappedVectorController(parent)
    {
    }

    void selected_setAabb(const ms8rect& aabb);
    void selected_setHitboxType(const EntityHitboxType& hitboxType);
};

class MetaSpriteController : public Controller::BaseController {
public:
    MetaSpriteController(Controller::ControllerInterface&);
    MetaSpriteController(const MetaSpriteController&) = delete;

    auto& frameSetController() { return _frameSetController; }
    auto& animationControllerInterface() { return _animationControllerInterface; }
    auto& paletteController() { return _paletteController; }
    auto& frameController() { return _frameController; }
    auto& frameObjectController() { return _frameObjectController; }
    auto& actionPointController() { return _actionPointController; }
    auto& entityHitboxController() { return _entityHitboxController; }

    auto& settingsController() { return _settingsController; }
    auto& selectedController() { return _selectedController; }

    virtual bool hasDocument() const final;

protected:
    virtual void doSave(const std::string& filename) final;
    virtual void doLoad(const std::string& filename) final;
    virtual void doNew() final;

private:
    FrameSetController _frameSetController;
    Animation::AnimationControllerImpl<FrameSetController> _animationControllerInterface;
    PaletteController _paletteController;
    FrameController _frameController;
    FrameObjectController _frameObjectController;
    ActionPointController _actionPointController;
    EntityHitboxController _entityHitboxController;

    ViewSettings::SettingsController _settingsController;
    SelectedController<MetaSpriteController> _selectedController;
};
}
}
}
