#pragma once

#include "animation.h"
#include "selected.h"
#include "settings.h"
#include "gui/controllers/basecontroller.h"
#include "gui/controllers/containers/cappedvectorcontroller.h"
#include "gui/controllers/containers/idmapcontroller.h"
#include "gui/controllers/containers/sharedptrrootcontroller.h"
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
    friend class FrameController;
    friend class PaletteController;
    friend class Animation::AnimationControllerImpl<FrameSetController>;

public:
    static const std::string HUMAN_TYPE_NAME;

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
    static const std::string HUMAN_TYPE_NAME;

public:
    PaletteController(FrameSetController& parent);

    // If < 0 then no color is selected.
    int selectedColorId() { return _selectedColorId; }
    void setSelectedColorId(unsigned colorId);
    void unsetSelectedColor();

    void selected_setColor(size_t index, const Snes::SnesColor& color);

    auto& signal_selectedColorChanged() { return _signal_selectedColorChanged; }

protected:
    virtual list_type* editable_listFromParent() final
    {
        FrameSet* fs = parent().editable_selected();
        return fs ? &fs->palettes : nullptr;
    }

private:
    int _selectedColorId;
    sigc::signal<void> _signal_selectedColorChanged;
};

class FrameController
    : public Controller::IdMapController<Frame, FrameSetController> {

    friend class FrameObjectController;
    friend class ActionPointController;
    friend class EntityHitboxController;

public:
    static const std::string HUMAN_TYPE_NAME;

public:
    FrameController(FrameSetController& parent)
        : IdMapController(parent)
    {
    }

    void selected_setSolid(const bool solid);
    void selected_setTileHitbox(const ms8rect& tileHitbox);

protected:
    virtual Frame::map_t* editable_mapFromParent() final
    {
        FrameSet* fs = parent().editable_selected();
        return fs ? &fs->frames : nullptr;
    }
};

class FrameObjectController
    : public Controller::CappedVectorController<FrameObject, FrameObject::list_t,
                                                FrameController> {

public:
    static const std::string HUMAN_TYPE_NAME;

public:
    FrameObjectController(FrameController& parent)
        : CappedVectorController(parent)
    {
    }

    void selected_setLocation(const ms8point& location);
    void selected_setTileId(const unsigned tileId);
    void selected_setSize(const ObjectSize size);
    void selected_setSizeAndTileId(const ObjectSize size, const unsigned tileId);
    void selected_setOrder(const unsigned order);
    void selected_setHFlip(const bool hFlip);
    void selected_setVFlip(const bool vFlip);

protected:
    virtual FrameObject::list_t* editable_listFromParent() final
    {
        Frame* f = parent().editable_selected();
        return f ? &f->objects : nullptr;
    }
};

class ActionPointController
    : public Controller::CappedVectorController<ActionPoint, ActionPoint::list_t,
                                                FrameController> {

public:
    static const std::string HUMAN_TYPE_NAME;

public:
    ActionPointController(FrameController& parent)
        : CappedVectorController(parent)
    {
    }

    void selected_setLocation(const ms8point& location);
    void selected_setParameter(const ActionPointParameter& parameter);

protected:
    virtual ActionPoint::list_t* editable_listFromParent() final
    {
        Frame* f = parent().editable_selected();
        return f ? &f->actionPoints : nullptr;
    }
};

class EntityHitboxController
    : public Controller::CappedVectorController<EntityHitbox, EntityHitbox::list_t,
                                                FrameController> {

public:
    static const std::string HUMAN_TYPE_NAME;

public:
    EntityHitboxController(FrameController& parent)
        : CappedVectorController(parent)
    {
    }

    void selected_setAabb(const ms8rect& aabb);
    void selected_setHitboxType(const EntityHitboxType& hitboxType);

protected:
    virtual EntityHitbox::list_t* editable_listFromParent() final
    {
        Frame* f = parent().editable_selected();
        return f ? &f->entityHitboxes : nullptr;
    }
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
