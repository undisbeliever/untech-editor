#include "metasprite.h"
#include "selected.hpp"
#include "gui/controllers/containers/cappedvectorcontroller.hpp"
#include "gui/controllers/containers/idmapcontroller.hpp"
#include "gui/controllers/containers/sharedptrrootcontroller.hpp"

using namespace UnTech;
using namespace UnTech::MetaSprite::MetaSprite;

using palette_list = capped_vector<Snes::Palette4bpp, MetaSprite::MAX_PALETTES>;

namespace UnTech {
namespace Controller {

template <>
palette_list& listFromParent<palette_list, FrameSet>(FrameSet& fs)
{
    return fs.palettes;
}

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
const std::string PaletteController::HUMAN_TYPE_NAME = "Palette";
const std::string FrameController::HUMAN_TYPE_NAME = "Frame";
const std::string FrameObjectController::HUMAN_TYPE_NAME = "Frame Object";
const std::string ActionPointController::HUMAN_TYPE_NAME = "Action Point";
const std::string EntityHitboxController::HUMAN_TYPE_NAME = "Entity Hitbox";

// MetaSpriteController
// ====================

template class MetaSprite::SelectedController<MetaSpriteController>;

MetaSpriteController::MetaSpriteController(Controller::ControllerInterface& interface)
    : BaseController(interface)
    , _frameSetController(*this)
    , _animationControllerInterface(_frameSetController)
    , _paletteController(_frameSetController)
    , _frameController(_frameSetController)
    , _frameObjectController(_frameController)
    , _actionPointController(_frameController)
    , _entityHitboxController(_frameController)
    , _settingsController()
    , _selectedController(*this)
{
}

bool MetaSpriteController::hasDocument() const
{
    return _frameSetController.hasSelected();
}

void MetaSpriteController::doSave(const std::string& filename)
{
    if (_frameSetController.hasSelected()) {
        saveFrameSet(_frameSetController.selected(), filename);
    }
}

void MetaSpriteController::doLoad(const std::string& filename)
{
    std::shared_ptr<FrameSet> fs = loadFrameSet(filename);

    _frameSetController.setRoot(fs);
}

void MetaSpriteController::doNew()
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

void FrameSetController::selected_addTiles(unsigned smallTiles, unsigned largeTiles)
{
    edit_selected(
        [&](auto&) { return smallTiles > 0 || smallTiles > 0; },
        [&](FrameSet& frameSet) {
            for (unsigned t = 0; t < smallTiles; t++) {
                frameSet.smallTileset.addTile();
            }
            for (unsigned t = 0; t < largeTiles; t++) {
                frameSet.largeTileset.addTile();
            }
        });
}

void FrameSetController::selected_smallTileset_setPixel(
    unsigned tileId, unsigned x, unsigned y, unsigned value)
{
    edit_selected(
        [&](const FrameSet& fs) {
            return fs.smallTileset.tile(tileId).pixel(x, y) != value;
        },
        [&](FrameSet& fs) {
            fs.smallTileset.tile(tileId).setPixel(x, y, value);
        });
}

void FrameSetController::selected_largeTileset_setPixel(
    unsigned tileId, unsigned x, unsigned y, unsigned value)
{
    edit_selected(
        [&](const FrameSet& fs) {
            return fs.largeTileset.tile(tileId).pixel(x, y) != value;
        },
        [&](FrameSet& fs) {
            fs.largeTileset.tile(tileId).setPixel(x, y, value);
        });
}

// PaletteController
// -----------------
template class Controller::CappedVectorController<Snes::Palette4bpp,
                                                  palette_list,
                                                  FrameSetController>;

PaletteController::PaletteController(FrameSetController& parent)
    : CappedVectorController(parent)
    , _selectedColorId(-1)
{
    // Ensure a palette is always selected.
    signal_selectedChanged().connect([this](void) {
        if (!hasSelected() && _list != nullptr && !_list->empty()) {
            selectIndex(0);
        }
    });
}

void PaletteController::setSelectedColorId(unsigned colorId)
{
    if (colorId < element_type::N_COLORS) {
        _selectedColorId = colorId;
        _signal_selectedColorChanged.emit();
    }
}

void PaletteController::unsetSelectedColor()
{
    _selectedColorId = -1;
    _signal_selectedColorChanged.emit();
}

void PaletteController::selected_setColor(size_t index, const Snes::SnesColor& color)
{
    if (index < element_type::N_COLORS) {
        edit_selected(
            [&](auto& palette) { return palette.color(index) != color; },
            [&](auto& palette) { palette.color(index) = color; });
    }
}

// FrameController
// ---------------
template class Controller::IdMapController<Frame, FrameSetController>;

void FrameController::selected_setSolid(const bool solid)
{
    edit_selected(
        [&](auto& frame) { return frame.solid != solid; },
        [&](auto& frame) { frame.solid = solid; });
}

void FrameController::selected_setTileHitbox(const ms8rect& tileHitbox)
{
    edit_selected(
        [&](auto& frame) { return frame.tileHitbox != tileHitbox; },
        [&](auto& frame) { frame.tileHitbox = tileHitbox; });
}

// FrameObjectController
// ---------------------
template class Controller::CappedVectorController<FrameObject, FrameObject::list_t,
                                                  FrameController>;

void FrameObjectController::selected_setLocation(const ms8point& location)
{
    edit_selected(
        [&](auto& obj) { return obj.location != location; },
        [&](auto& obj) { obj.location = location; });
}

void FrameObjectController::selected_setTileId(const unsigned tileId)
{
    edit_selected(
        [&](auto& obj) {
            const FrameSet& frameSet = this->parent().parent().selected();

            if (obj.tileId == tileId) {
                return false;
            }
            if (obj.size == ObjectSize::SMALL) {
                return tileId < frameSet.smallTileset.size();
            }
            else {
                return tileId < frameSet.largeTileset.size();
            }
        },
        [&](auto& obj) {
            obj.tileId = tileId;
        });
}

void FrameObjectController::selected_setSize(const ObjectSize size)
{
    edit_selected(
        [&](auto& obj) { return obj.size != size; },
        [&](auto& obj) {
            // updating size can cause the tileId to be invalid
            const FrameSet& frameSet = this->parent().parent().selected();

            obj.size = size;

            if (size == ObjectSize::SMALL) {
                if (obj.tileId >= frameSet.smallTileset.size()) {
                    obj.tileId = 0;
                }
            }
            else {
                if (obj.tileId >= frameSet.largeTileset.size()) {
                    obj.tileId = 0;
                }
            }
        });
}

void FrameObjectController::selected_setSizeAndTileId(
    const ObjectSize size, const unsigned tileId)
{
    edit_selected(
        [&](auto& obj) {
            const FrameSet& frameSet = this->parent().parent().selected();

            if (obj.size == size && obj.tileId == tileId) {
                return false;
            }
            if (size == ObjectSize::SMALL) {
                return tileId < frameSet.smallTileset.size();
            }
            else {
                return tileId < frameSet.largeTileset.size();
            }
        },
        [&](auto& obj) {
            obj.size = size;
            obj.tileId = tileId;
        });
}

void FrameObjectController::selected_setOrder(const unsigned order)
{
    edit_selected(
        [&](auto& obj) { return obj.order != order; },
        [&](auto& obj) { obj.order = order; });
}

void FrameObjectController::selected_setHFlip(const bool hFlip)
{
    edit_selected(
        [&](auto& obj) { return obj.hFlip != hFlip; },
        [&](auto& obj) { obj.hFlip = hFlip; });
}

void FrameObjectController::selected_setVFlip(const bool vFlip)
{
    edit_selected(
        [&](auto& obj) { return obj.vFlip != vFlip; },
        [&](auto& obj) { obj.vFlip = vFlip; });
}

// ActionPointController
// ---------------------
template class Controller::CappedVectorController<ActionPoint, ActionPoint::list_t,
                                                  FrameController>;

void ActionPointController::selected_setLocation(const ms8point& location)
{
    edit_selected(
        [&](auto& ap) { return ap.location != location; },
        [&](auto& ap) { ap.location = location; });
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

void EntityHitboxController::selected_setAabb(const ms8rect& aabb)
{
    edit_selected(
        [&](auto& eh) { return eh.aabb != aabb; },
        [&](auto& eh) { eh.aabb = aabb; });
}

void EntityHitboxController::selected_setHitboxType(const EntityHitboxType& hitboxType)
{
    edit_selected(
        [&](auto& eh) { return eh.hitboxType != hitboxType; },
        [&](auto& eh) { eh.hitboxType = hitboxType; });
}
