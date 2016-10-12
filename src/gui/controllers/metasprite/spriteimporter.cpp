#include "spriteimporter.h"
#include "selected.hpp"
#include "gui/controllers/containers/cappedvectorcontroller.hpp"
#include "gui/controllers/containers/idmapcontroller.hpp"
#include "gui/controllers/containers/sharedptrrootcontroller.hpp"

using namespace UnTech;
using namespace UnTech::MetaSprite::SpriteImporter;

namespace UnTech {
namespace Controller {
template <>
idmap<Frame>& idmapFromParent<Frame, FrameSet>(FrameSet& fs)
{
    return fs.frames;
}

template <>
FrameObject::list_t& listFromParent<FrameObject::list_t, Frame>(Frame& f)
{
    return f.objects;
}

template <>
ActionPoint::list_t& listFromParent<ActionPoint::list_t, Frame>(Frame& f)
{
    return f.actionPoints;
}

template <>
EntityHitbox::list_t& listFromParent<EntityHitbox::list_t, Frame>(Frame& f)
{
    return f.entityHitboxes;
}
}
}

const std::string FrameSetController::HUMAN_TYPE_NAME = "Frame Set";
const std::string FrameController::HUMAN_TYPE_NAME = "Frame";
const std::string FrameObjectController::HUMAN_TYPE_NAME = "Frame Object";
const std::string ActionPointController::HUMAN_TYPE_NAME = "Action Point";
const std::string EntityHitboxController::HUMAN_TYPE_NAME = "Entity Hitbox";

// SpriteImporterController
// ========================

template class MetaSprite::SelectedController<SpriteImporterController>;

SpriteImporterController::SpriteImporterController(Controller::ControllerInterface& interface)
    : BaseController(interface)
    , _frameSetController(*this)
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
    edit_selected(
        [&](auto& fs) { return fs.name != name; },
        [&](auto& fs) { fs.name = name; });
}

void FrameSetController::selected_setTilesetType(const TilesetType tilesetType)
{
    edit_selected(
        [&](auto& fs) { return fs.tilesetType != tilesetType; },
        [&](auto& fs) { fs.tilesetType = tilesetType; });
}

void FrameSetController::selected_setExportOrderFilename(const std::string& filename)
{
    edit_selected(
        [&](auto) { return !filename.empty(); },
        [&](auto& fs) {
            fs.exportOrder = loadFrameSetExportOrderCached(filename);
        });
}

void FrameSetController::selected_setImageFilename(const std::string& filename)
{
    edit_selected(
        [&](auto& fs) { return fs.imageFilename != filename; },
        [&](auto& fs) { fs.loadImage(filename); });
}

void FrameSetController::selected_setTransparentColor(const rgba& color)
{
    edit_selected(
        [&](auto& fs) { return fs.transparentColor != color; },
        [&](auto& fs) { fs.transparentColor = color; });
}

void FrameSetController::selected_setGrid(const FrameSetGrid& grid)
{
    edit_selected(
        [&](auto& frameSet) { return grid.isValid(frameSet); },
        [&](auto& frameSet) {
            frameSet.grid = grid;
            frameSet.updateFrameLocations();
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
    const FrameSet& fs = parent().selected();

    FrameLocation loc = location;
    loc.update(fs.grid, this->selected());

    edit_selected(
        [&](auto& frame) { return frame.location != loc; },
        [&](auto& frame) { frame.location = loc; });
}

void FrameController::selected_setTileHitbox(const urect& tileHitbox)
{
    const Frame& f = selected();
    auto th = f.location.aabb.clipInside(tileHitbox, f.tileHitbox);

    edit_selected(
        [&](auto& frame) { return frame.tileHitbox != th; },
        [&](auto& frame) { frame.tileHitbox = th; });
}

void FrameController::selected_setSpriteOrder(const SpriteOrderType& spriteOrder)
{
    edit_selected(
        [&](auto& frame) { return frame.spriteOrder != spriteOrder; },
        [&](auto& frame) { frame.spriteOrder = spriteOrder; });
}

void FrameController::selected_setSolid(const bool solid)
{
    edit_selected(
        [&](auto& frame) { return frame.solid != solid; },
        [&](auto& frame) { frame.solid = solid; });
}

// FrameObjectController
// ---------------------
template class Controller::CappedVectorController<FrameObject, FrameObject::list_t,
                                                  FrameController>;

void FrameObjectController::selected_setLocation(const upoint& location)
{
    const urect& frameAabb = parent().selected().location.aabb;
    const auto objSize = selected().sizePx();
    const upoint loc = frameAabb.clipInside(location, objSize);

    edit_selected(
        [&](auto& obj) { return obj.location != loc; },
        [&](auto& obj) { obj.location = loc; });
}

void FrameObjectController::selected_setSize(const ObjectSize size)
{
    edit_selected(
        [&](auto& obj) { return obj.size != size; },
        [&](auto& obj) {
            // updating size can cause location to appear outside frame aabb
            const Frame& frame = this->parent().selected();

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
    const urect& frameAabb = parent().selected().location.aabb;
    const upoint loc = frameAabb.clipInside(location);

    edit_selected(
        [&](auto& ap) { return ap.location != loc; },
        [&](auto& ap) { ap.location = loc; });
}

void ActionPointController::selected_setParameter(const ActionPointParameter& parameter)
{
    edit_selected(
        [&](auto& ap) { return ap.parameter != parameter; },
        [&](auto& ap) { ap.parameter = parameter; });
}

// EntityHitboxController
// ----------------------
template class Controller::CappedVectorController<EntityHitbox, EntityHitbox::list_t,
                                                  FrameController>;

void EntityHitboxController::selected_setAabb(const urect& aabb)
{
    const urect& frameAabb = parent().selected().location.aabb;
    const urect& oldAabb = selected().aabb;
    const urect newAabb = frameAabb.clipInside(aabb, oldAabb);

    edit_selected(
        [&](auto& eh) { return eh.aabb != newAabb; },
        [&](auto& eh) { eh.aabb = newAabb; });
}

void EntityHitboxController::selected_setHitboxType(const EntityHitboxType& hitboxType)
{
    edit_selected(
        [&](auto& eh) { return eh.hitboxType != hitboxType; },
        [&](auto& eh) { eh.hitboxType = hitboxType; });
}
