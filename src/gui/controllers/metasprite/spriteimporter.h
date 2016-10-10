#pragma once

#include "animation.h"
#include "selected.h"
#include "settings.h"
#include "gui/controllers/basecontroller.h"
#include "gui/controllers/containers/cappedvectorcontroller.h"
#include "gui/controllers/containers/idmapcontroller.h"
#include "gui/controllers/containers/sharedptrrootcontroller.h"
#include "models/metasprite/spriteimporter.h"

namespace UnTech {
namespace MetaSprite {
namespace SpriteImporter {

class SpriteImporterController;
class FrameController;
class FrameObjectController;
class ActionPointController;
class EntityHitboxController;

class FrameSetController
    : public Controller::SharedPtrRootController<FrameSet> {

    friend class SpriteImporterController;
    friend class FrameController;
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

    void selected_setImageFilename(const std::string& filename);
    void selected_setTransparentColor(const rgba& color);
    void selected_setGrid(const FrameSetGrid& grid);

    void selected_reloadImage();
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

    void selected_setLocation(const FrameLocation& location);
    void selected_setTileHitbox(const urect& tileHitbox);
    void selected_setSpriteOrder(const SpriteOrderType& spriteOrder);
    void selected_setSolid(const bool solid);

protected:
    virtual Frame::map_t* editable_mapFromParent() final
    {
        FrameSet* fs = parent().editable_selected();
        return fs ? &fs->frames : nullptr;
    }

    virtual void onCreate(const idstring& id, Frame& frame);
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

    void selected_setLocation(const upoint& location);
    void selected_setSize(const ObjectSize size);

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

    void selected_setLocation(const upoint& location);
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

    void selected_setAabb(const urect& aabb);
    void selected_setHitboxType(const EntityHitboxType& hitboxType);

protected:
    virtual EntityHitbox::list_t* editable_listFromParent() final
    {
        Frame* f = parent().editable_selected();
        return f ? &f->entityHitboxes : nullptr;
    }
};

class SpriteImporterController : public Controller::BaseController {
public:
    SpriteImporterController(Controller::ControllerInterface&);
    SpriteImporterController(const SpriteImporterController&) = delete;

    auto& frameSetController() { return _frameSetController; }
    auto& animationControllerInterface() { return _animationControllerInterface; }
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
    FrameController _frameController;
    FrameObjectController _frameObjectController;
    ActionPointController _actionPointController;
    EntityHitboxController _entityHitboxController;

    ViewSettings::SettingsController _settingsController;
    SelectedController<SpriteImporterController> _selectedController;
};
}
}
}
