#include "spriteimporter.h"

using namespace UnTech::MetaSprite::SpriteImporter;

SpriteImporterController::SpriteImporterController()
    : _frameSetController()
    , _animationControllerInterface(_frameSetController)
    , _frameController(_frameSetController)
    , _frameObjectController(_frameController)
    , _actionPointController(_frameController)
    , _entityHitboxController(_frameController)
    , _settingsController()
{
}

const std::string FrameSetController::HUMAN_TYPE_NAME = "Frame Set";
const std::string FrameController::HUMAN_TYPE_NAME = "Frame";
const std::string FrameObjectController::HUMAN_TYPE_NAME = "Frame Object";
const std::string ActionPointController::HUMAN_TYPE_NAME = "Action Point";
const std::string EntityHitboxController::HUMAN_TYPE_NAME = "Entity Hitbox";

// FrameSetController
// ------------------

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
    }
}

// FrameController
// ---------------

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
