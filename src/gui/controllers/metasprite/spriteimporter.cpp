#include "spriteimporter.h"
#include "selected.hpp"
#include "gui/controllers/containers/cappedvectorcontroller.hpp"
#include "gui/controllers/containers/idmapcontroller.hpp"
#include "gui/controllers/containers/sharedptrrootcontroller.hpp"

using namespace UnTech;
using namespace UnTech::MetaSprite::SpriteImporter;

const std::string FrameSetController::HUMAN_TYPE_NAME = "Frame Set";
const std::string FrameController::HUMAN_TYPE_NAME = "Frame";
const std::string FrameObjectController::HUMAN_TYPE_NAME = "Frame Object";
const std::string ActionPointController::HUMAN_TYPE_NAME = "Action Point";
const std::string EntityHitboxController::HUMAN_TYPE_NAME = "Entity Hitbox";

template class MetaSprite::SelectedController<SpriteImporterController>;

SpriteImporterController::SpriteImporterController(Controller::ControllerInterface& interface)
    : BaseController(interface)
    , _frameSetController()
    , _animationControllerInterface(_frameSetController)
    , _frameController(_frameSetController)
    , _frameObjectController(_frameController)
    , _actionPointController(_frameController)
    , _entityHitboxController(_frameController)
    , _settingsController()
    , _selectedController(*this)
{
}

bool SpriteImporterController::hasDocument() const
{
    return _frameSetController.hasSelected();
}

void SpriteImporterController::doSave(const std::string& filename)
{
    if (_frameSetController.hasSelected()) {
        saveFrameSet(_frameSetController.selected(), filename);
    }
}

void SpriteImporterController::doLoad(const std::string& filename)
{
    std::shared_ptr<FrameSet> fs = loadFrameSet(filename);

    _frameSetController.setRoot(fs);
}

void SpriteImporterController::doNew()
{
    auto frameSet = std::make_shared<FrameSet>();
    _frameSetController.setRoot(frameSet);
}

// FrameSetController
// ------------------
template class Controller::SharedPtrRootController<FrameSet>;

void FrameSetController::selected_setName(const idstring& name)
{
    edit_selected([&](FrameSet& frameSet) {
        frameSet.name = name;
    });
}

void FrameSetController::selected_setTilesetType(const TilesetType tilesetType)
{
    edit_selected([&](FrameSet& frameSet) {
        frameSet.tilesetType = tilesetType;
    });
}

void FrameSetController::selected_setExportOrderFilename(const std::string& filename)
{
    edit_selected([&](FrameSet& frameSet) {
        frameSet.exportOrder = loadFrameSetExportOrderCached(filename);
    });
}

void FrameSetController::selected_setImageFilename(const std::string& filename)
{
    edit_selected([&](FrameSet& frameSet) {
        frameSet.loadImage(filename);
    });
}

void FrameSetController::selected_setTransparentColor(const rgba& color)
{
    edit_selected([&](FrameSet& frameSet) {
        frameSet.transparentColor = color;
    });
}

void FrameSetController::selected_setGrid(const FrameSetGrid& grid)
{
    edit_selected([&](FrameSet& frameSet) {
        if (grid.isValid(frameSet)) {
            frameSet.grid = grid;
            frameSet.updateFrameLocations();
        }
    });
}

void FrameSetController::selected_reloadImage()
{
    // Not an undo-able action.
    FrameSet* frameSet = editable_selected();

    if (frameSet) {
        frameSet->reloadImage();

        signal_dataChanged().emit();
        signal_anyChanged().emit();
    }
}

// FrameController
// ---------------
template class Controller::IdMapController<Frame, FrameSetController>;

void FrameController::onCreate(const idstring&, Frame& frame)
{
    const FrameSet& fs = this->parent().selected();

    frame.location.useGridLocation = true;
    frame.location.useGridOrigin = true;

    frame.location.update(fs.grid, frame);
}

void FrameController::selected_setLocation(const FrameLocation& location)
{
    edit_selected([&](Frame& frame) {
        const FrameSet& fs = this->parent().selected();

        frame.location = location;
        frame.location.update(fs.grid, frame);
    });
}

void FrameController::selected_setTileHitbox(const urect& tileHitbox)
{
    edit_selected([&](Frame& frame) {
        frame.tileHitbox = frame.location.aabb.clipInside(tileHitbox, frame.tileHitbox);
    });
}

void FrameController::selected_setSpriteOrder(const SpriteOrderType& spriteOrder)
{
    edit_selected([&](Frame& frame) {
        frame.spriteOrder = spriteOrder;
    });
}

void FrameController::selected_setSolid(const bool solid)
{
    edit_selected([&](Frame& frame) {
        frame.solid = solid;
    });
}

// FrameObjectController
// ---------------------
template class Controller::CappedVectorController<FrameObject, FrameObject::list_t,
                                                  FrameController>;

void FrameObjectController::selected_setLocation(const upoint& location)
{
    edit_selected([&](FrameObject& obj) {
        const Frame& frame = parent().selected();
        obj.location = frame.location.aabb.clipInside(location, obj.sizePx());
    });
}

void FrameObjectController::selected_setSize(const ObjectSize size)
{
    edit_selected([&](FrameObject& obj) {
        const Frame& frame = parent().selected();

        // updating size can cause location to appear outside frame aabb
        obj.size = size;
        obj.location = frame.location.aabb.clipInside(obj.location, obj.sizePx());
    });
}

// ActionPointController
// ---------------------
template class Controller::CappedVectorController<ActionPoint, ActionPoint::list_t,
                                                  FrameController>;

void ActionPointController::selected_setLocation(const upoint& location)
{
    edit_selected([&](ActionPoint& ap) {
        const Frame& frame = parent().selected();
        ap.location = frame.location.aabb.clipInside(location);
    });
}

void ActionPointController::selected_setParameter(const ActionPointParameter& parameter)
{
    edit_selected([&](ActionPoint& ap) {
        ap.parameter = parameter;
    });
}

// EntityHitboxController
// ----------------------
template class Controller::CappedVectorController<EntityHitbox, EntityHitbox::list_t,
                                                  FrameController>;

void EntityHitboxController::selected_setAabb(const urect& aabb)
{
    edit_selected([&](EntityHitbox& eh) {
        const Frame& frame = parent().selected();
        eh.aabb = frame.location.aabb.clipInside(aabb, eh.aabb);
    });
}

void EntityHitboxController::selected_setHitboxType(const EntityHitboxType& hitboxType)
{
    edit_selected([&](EntityHitbox& eh) {
        eh.hitboxType = hitboxType;
    });
}
