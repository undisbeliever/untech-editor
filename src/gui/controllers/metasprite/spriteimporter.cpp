/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "spriteimporter.h"
#include "selected.hpp"
#include "gui/controllers/containers/cappedvectorcontroller.hpp"
#include "gui/controllers/containers/idmapcontroller.hpp"
#include "gui/controllers/containers/sharedptrrootcontroller.hpp"

using namespace UnTech;
using namespace UnTech::MetaSprite::SpriteImporter;
using UndoActionType = UnTech::Controller::Undo::ActionType;

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
    const static UndoActionType actionType = { "Edit FrameSet Name", false };

    edit_selected(
        &actionType,
        [&](auto& fs) { return fs.name != name; },
        [&](auto& fs) { fs.name = name; });
}

void FrameSetController::selected_setTilesetType(const TilesetType tilesetType)
{
    const static UndoActionType actionType = { "Edit Tileset Type", false };

    edit_selected(
        &actionType,
        [&](auto& fs) { return fs.tilesetType != tilesetType; },
        [&](auto& fs) { fs.tilesetType = tilesetType; });
}

void FrameSetController::selected_setExportOrderFilename(const std::string& filename)
{
    const static UndoActionType actionType = { "Set Export Order File", false };

    edit_selected(
        &actionType,
        [&](auto) { return !filename.empty(); },
        [&](auto& fs) {
            fs.exportOrder = loadFrameSetExportOrderCached(filename);
        });
}

void FrameSetController::selected_setImageFilename(const std::string& filename)
{
    const static UndoActionType actionType = { "Set FrameSet Image", false };

    edit_selected(
        &actionType,
        [&](auto& fs) { return fs.imageFilename != filename; },
        [&](auto& fs) { fs.loadImage(filename); });
}

void FrameSetController::selected_setTransparentColor(const rgba& color)
{
    const static UndoActionType actionType = { "Set Transparent Color", false };

    edit_selected(
        &actionType,
        [&](auto& fs) { return fs.transparentColor != color; },
        [&](auto& fs) { fs.transparentColor = color; });
}

void FrameSetController::selected_setGrid(const FrameSetGrid& grid)
{
    const static UndoActionType actionType = { "Edit FrameSet Grid", true };

    edit_selected(
        &actionType,
        [&](auto& frameSet) { return grid.isValid(frameSet); },
        [&](auto& frameSet) {
            frameSet.grid = grid;
            frameSet.updateFrameLocations();
        });
}

void FrameSetController::selected_setPalette(const UserSuppliedPalette& palette)
{
    const static UndoActionType actionType = { "Edit User Supplied Palette", true };

    edit_selected(
        &actionType,
        [&](auto& frameSet) { return frameSet.palette != palette; },
        [&](auto& frameSet) {
            frameSet.palette = palette;
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
    const static UndoActionType actionType = { "Edit Frame Location", true };

    const FrameSet& fs = parent().selected();

    FrameLocation loc = location;
    loc.update(fs.grid, this->selected());

    edit_selected(
        &actionType,
        [&](auto& frame) { return frame.location != loc; },
        [&](auto& frame) { frame.location = loc; });
}

void FrameController::selected_setTileHitbox(const urect& tileHitbox)
{
    const static UndoActionType actionType = { "Edit Tile Hitbox", true };

    const Frame& f = selected();
    auto th = f.location.aabb.clipInside(tileHitbox, f.tileHitbox);

    edit_selected(
        &actionType,
        [&](auto& frame) { return frame.tileHitbox != th; },
        [&](auto& frame) { frame.tileHitbox = th; });
}

void FrameController::selected_setSpriteOrder(const SpriteOrderType& spriteOrder)
{
    const static UndoActionType actionType = { "Edit Frame Sprite Order", false };

    edit_selected(
        &actionType,
        [&](auto& frame) { return frame.spriteOrder != spriteOrder; },
        [&](auto& frame) { frame.spriteOrder = spriteOrder; });
}

void FrameController::selected_setSolid(const bool solid)
{
    const static UndoActionType actionType[2] = {
        { "Set Frame Solid", false },
        { "Set Frame Not-Solid", false },
    };

    edit_selected(
        &actionType[solid],
        [&](auto& frame) { return frame.solid != solid; },
        [&](auto& frame) { frame.solid = solid; });
}

// FrameObjectController
// ---------------------
template class Controller::CappedVectorController<FrameObject, FrameObject::list_t,
                                                  FrameController>;

void FrameObjectController::selected_setLocation(const upoint& location)
{
    const static UndoActionType actionType = { "Edit Object Location", true };

    const urect& frameAabb = parent().selected().location.aabb;
    const auto objSize = selected().sizePx();
    const upoint loc = frameAabb.clipInside(location, objSize);

    edit_selected(
        &actionType,
        [&](auto& obj) { return obj.location != loc; },
        [&](auto& obj) { obj.location = loc; });
}

void FrameObjectController::selected_setSize(const ObjectSize size)
{
    const static UndoActionType actionType = { "Edit Object Size", false };

    edit_selected(
        &actionType,
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
    const static UndoActionType actionType = { "Edit Action Point Location", true };

    const urect& frameAabb = parent().selected().location.aabb;
    const upoint loc = frameAabb.clipInside(location);

    edit_selected(
        &actionType,
        [&](auto& ap) { return ap.location != loc; },
        [&](auto& ap) { ap.location = loc; });
}

void ActionPointController::selected_setParameter(const ActionPointParameter& parameter)
{
    const static UndoActionType actionType = { "Edit Action Point Parameter", true };

    edit_selected(
        &actionType,
        [&](auto& ap) { return ap.parameter != parameter; },
        [&](auto& ap) { ap.parameter = parameter; });
}

// EntityHitboxController
// ----------------------
template class Controller::CappedVectorController<EntityHitbox, EntityHitbox::list_t,
                                                  FrameController>;

void EntityHitboxController::selected_setAabb(const urect& aabb)
{
    const static UndoActionType actionType = { "Edit Entity Hitbox Aabb", true };

    const urect& frameAabb = parent().selected().location.aabb;
    const urect& oldAabb = selected().aabb;
    const urect newAabb = frameAabb.clipInside(aabb, oldAabb);

    edit_selected(
        &actionType,
        [&](auto& eh) { return eh.aabb != newAabb; },
        [&](auto& eh) { eh.aabb = newAabb; });
}

void EntityHitboxController::selected_setHitboxType(const EntityHitboxType& hitboxType)
{
    const static UndoActionType actionType = { "Edit Entity Hitbox Type", false };

    edit_selected(
        &actionType,
        [&](auto& eh) { return eh.hitboxType != hitboxType; },
        [&](auto& eh) { eh.hitboxType = hitboxType; });
}
