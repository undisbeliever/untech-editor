#include "metasprite.h"
#include "selected.hpp"
#include "gui/controllers/containers/cappedvectorcontroller.hpp"
#include "gui/controllers/containers/idmapcontroller.hpp"
#include "gui/controllers/containers/sharedptrrootcontroller.hpp"

using namespace UnTech;
using namespace UnTech::MetaSprite::MetaSprite;

const std::string FrameSetController::HUMAN_TYPE_NAME = "Frame Set";
const std::string PaletteController::HUMAN_TYPE_NAME = "Palette";
const std::string FrameController::HUMAN_TYPE_NAME = "Frame";
const std::string FrameObjectController::HUMAN_TYPE_NAME = "Frame Object";
const std::string ActionPointController::HUMAN_TYPE_NAME = "Action Point";
const std::string EntityHitboxController::HUMAN_TYPE_NAME = "Entity Hitbox";

template class MetaSprite::SelectedController<MetaSpriteController>;

MetaSpriteController::MetaSpriteController(Controller::ControllerInterface& interface)
    : BaseController(interface)
    , _frameSetController()
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

void FrameSetController::selected_addTiles(unsigned smallTiles, unsigned largeTiles)
{
    edit_selected([&](FrameSet& frameSet) {
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
    edit_selected([&](FrameSet& frameSet) {
        frameSet.smallTileset.tile(tileId).setPixel(x, y, value);
    });
}

void FrameSetController::selected_largeTileset_setPixel(
    unsigned tileId, unsigned x, unsigned y, unsigned value)
{
    edit_selected([&](FrameSet& frameSet) {
        frameSet.largeTileset.tile(tileId).setPixel(x, y, value);
    });
}

// PaletteController
// -----------------
template class Controller::CappedVectorController<Snes::Palette4bpp,
                                                  capped_vector<Snes::Palette4bpp, MetaSprite::MAX_PALETTES>,
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
        edit_selected([&](Snes::Palette4bpp& palette) {
            palette.color(index) = color;
        });
    }
}

// FrameController
// ---------------
template class Controller::IdMapController<Frame, FrameSetController>;

void FrameController::selected_setSolid(const bool solid)
{
    edit_selected([&](Frame& frame) {
        frame.solid = solid;
    });
}

void FrameController::selected_setTileHitbox(const ms8rect& tileHitbox)
{
    edit_selected([&](Frame& frame) {
        frame.tileHitbox = tileHitbox;
    });
}

// FrameObjectController
// ---------------------
template class Controller::CappedVectorController<FrameObject, FrameObject::list_t,
                                                  FrameController>;

void FrameObjectController::selected_setLocation(const ms8point& location)
{
    edit_selected([&](FrameObject& obj) {
        obj.location = location;
    });
}

void FrameObjectController::selected_setTileId(const unsigned tileId)
{
    edit_selected([&](FrameObject& obj) {
        const FrameSet& frameset = parent().parent().selected();

        if (obj.size == ObjectSize::SMALL) {
            obj.tileId = std::min<unsigned>(tileId, frameset.smallTileset.size());
        }
        else {
            obj.tileId = std::min<unsigned>(tileId, frameset.largeTileset.size());
        }
    });
}

void FrameObjectController::selected_setSize(const ObjectSize size)
{
    edit_selected([&](FrameObject& obj) {
        obj.size = size;
    });
}

void FrameObjectController::selected_setSizeAndTileId(
    const ObjectSize size, const unsigned tileId)
{
    edit_selected([&](FrameObject& obj) {
        const FrameSet& frameset = parent().parent().selected();

        obj.size = size;

        if (size == ObjectSize::SMALL) {
            obj.tileId = std::min<unsigned>(tileId, frameset.smallTileset.size());
        }
        else {
            obj.tileId = std::min<unsigned>(tileId, frameset.largeTileset.size());
        }
    });
}

void FrameObjectController::selected_setOrder(const unsigned order)
{
    edit_selected([&](FrameObject& obj) {
        obj.order = order;
    });
}

void FrameObjectController::selected_setHFlip(const bool hFlip)
{
    edit_selected([&](FrameObject& obj) {
        obj.hFlip = hFlip;
    });
}

void FrameObjectController::selected_setVFlip(const bool vFlip)
{
    edit_selected([&](FrameObject& obj) {
        obj.vFlip = vFlip;
    });
}

// ActionPointController
// ---------------------
template class Controller::CappedVectorController<ActionPoint, ActionPoint::list_t,
                                                  FrameController>;

void ActionPointController::selected_setLocation(const ms8point& location)
{
    edit_selected([&](ActionPoint& ap) {
        ap.location = location;
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

void EntityHitboxController::selected_setAabb(const ms8rect& aabb)
{
    edit_selected([&](EntityHitbox& ap) {
        ap.aabb = aabb;
    });
}

void EntityHitboxController::selected_setHitboxType(const EntityHitboxType& hitboxType)
{
    edit_selected([&](EntityHitbox& ap) {
        ap.hitboxType = hitboxType;
    });
}
