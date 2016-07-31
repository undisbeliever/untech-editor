#include "metasprite.h"
#include "gui/controllers/helpers/actionhelper.h"
#include "gui/controllers/helpers/documentcontroller.hpp"
#include "gui/controllers/helpers/mergeactionhelper.h"
#include "gui/controllers/helpers/namedlistcontroller.hpp"
#include "gui/controllers/helpers/orderedlistcontroller.hpp"
#include "gui/controllers/helpers/spriteselectedtypecontroller.hpp"
#include "models/metasprite.h"

using namespace UnTech::MetaSprite;

template class UnTech::Controller::SpriteSelectedTypeController<MetaSpriteController>;

/*
 * META SPRITE CONTROLLER
 * ======================
 */
template class UnTech::Controller::DocumentController<MetaSpriteDocument>;

MetaSpriteController::MetaSpriteController(std::unique_ptr<Controller::ControllerInterface> interface)
    : DocumentController(std::move(interface))
    , _frameSetController(*this)
    , _abstractFrameSetController(*this)
    , _paletteController(*this)
    , _frameController(*this)
    , _entityHitboxController(*this)
    , _actionPointController(*this)
    , _frameObjectController(*this)
    , _layersController()
    , _selectedTypeController(*this)
{

    /*
     * SIGNALS
     * -------
     */
    this->signal_documentChanged().connect([this](void) {
        auto* document = this->document_editable();
        if (document != nullptr) {
            _frameSetController.setSelected(&document->frameSet());
            _abstractFrameSetController.setSelected(&document->frameSet());
        }
        else {
            _frameSetController.setSelected(nullptr);
            _abstractFrameSetController.setSelected(nullptr);
        }
    });
    _frameSetController.signal_selectedChanged().connect([this](void) {
        auto* frameSet = _frameSetController.selected_editable();
        if (frameSet != nullptr) {
            _frameController.setList(&frameSet->frames());
            _paletteController.setList(&frameSet->palettes());
        }
        else {
            _frameController.setList(nullptr);
            _paletteController.setList(nullptr);
        }
    });
    _frameController.signal_selectedChanged().connect([this](void) {
        auto* frame = _frameController.selected_editable();
        if (frame != nullptr) {
            _entityHitboxController.setList(&frame->entityHitboxes());
            _actionPointController.setList(&frame->actionPoints());
            _frameObjectController.setList(&frame->objects());
        }
        else {
            _entityHitboxController.setList(nullptr);
            _actionPointController.setList(nullptr);
            _frameObjectController.setList(nullptr);
        }
    });
    _paletteController.signal_listChanged().connect([this](void) {
        // If no palette is selected and there are palettes available,
        // then select the first one automatically.

        auto* palette = _paletteController.selected();
        auto* paletteList = _paletteController.list();

        if (palette == nullptr && paletteList != nullptr && paletteList->size() > 0) {
            _paletteController.setSelected(&*paletteList->begin());
        }
    });
    _paletteController.signal_selectedChanged().connect([this](void) {
        auto* palette = _paletteController.selected();
        auto* paletteList = _paletteController.list();

        if (palette == nullptr && paletteList != nullptr && paletteList->size() > 0) {
            _paletteController.setSelected(&*paletteList->begin());
        }
    });
}

void MetaSpriteController::emitAllDataChanged()
{
    _abstractFrameSetController.emitAllDataChanged();

    _frameSetController.signal_selectedChanged().emit();
    _paletteController.signal_listChanged().emit();
    _paletteController.signal_selectedChanged().emit();
    _frameController.signal_listChanged().emit();
    _frameController.signal_selectedChanged().emit();
    _entityHitboxController.signal_listChanged().emit();
    _entityHitboxController.signal_selectedChanged().emit();
    _actionPointController.signal_listChanged().emit();
    _actionPointController.signal_selectedChanged().emit();
    _frameObjectController.signal_listChanged().emit();
    _frameObjectController.signal_selectedChanged().emit();
    _selectedTypeController.signal_typeChanged().emit();
}

/*
 * FRAME OBJECT CONTROLLER
 * =======================
 */
template class UnTech::Controller::OrderedListController<FrameObject>;

CREATE_SIMPLE_ACTION(FrameObjectController, selected_setLocation,
                     FrameObject, UnTech::ms8point, location, setLocation,
                     signal_dataChanged,
                     "Set Frame Object Location")

CREATE_SIMPLE_ACTION(FrameObjectController, selected_setTileId,
                     FrameObject, unsigned, tileId, setTileId,
                     signal_dataChanged,
                     "Set Frame Object Tile")

CREATE_SIMPLE_ACTION2(FrameObjectController, selected_setSize,
                      FrameObject, FrameObject::ObjectSize, size, setSize,
                      signal_dataChanged, signal_sizeChanged,
                      "Change Frame Object Size")

CREATE_DUAL_ACTION2(FrameObjectController, selected_setSizeAndTileId,
                    FrameObject,
                    FrameObject::ObjectSize, size, setSize,
                    unsigned, tileId, setTileId,
                    signal_dataChanged, signal_sizeChanged,
                    "Set Frame Object Location")

CREATE_SIMPLE_ACTION(FrameObjectController, selected_setOrder,
                     FrameObject, unsigned, order, setOrder,
                     signal_dataChanged,
                     "Change Frame Object Order")

CREATE_SIMPLE_ACTION(FrameObjectController, selected_setHFlip,
                     FrameObject, bool, hFlip, setHFlip,
                     signal_dataChanged,
                     "Change Frame Object hFlip")

CREATE_SIMPLE_ACTION(FrameObjectController, selected_setVFlip,
                     FrameObject, bool, vFlip, setVFlip,
                     signal_dataChanged,
                     "Change Frame Object vFlip")

CREATE_MERGE_ACTION(FrameObjectController, selected_setLocation_merge,
                    FrameObject, UnTech::ms8point, location, setLocation,
                    signal_dataChanged,
                    "Set Frame Object Location")

CREATE_MERGE_ACTION(FrameObjectController, selected_setTileId_merge,
                    FrameObject, unsigned, tileId, setTileId,
                    signal_dataChanged,
                    "Set Frame Object Tile")

/*
 * ACTION POINT CONTROLLER
 * =======================
 */
template class UnTech::Controller::OrderedListController<ActionPoint>;

CREATE_SIMPLE_ACTION(ActionPointController, selected_setLocation,
                     ActionPoint, UnTech::ms8point, location, setLocation,
                     signal_dataChanged,
                     "Set Action Point Location")

CREATE_SIMPLE_ACTION(ActionPointController, selected_setParameter,
                     ActionPoint, unsigned, parameter, setParameter,
                     signal_dataChanged,
                     "Set Action Point Parameter")

CREATE_MERGE_ACTION(ActionPointController, selected_setLocation_merge,
                    ActionPoint, UnTech::ms8point, location, setLocation,
                    signal_dataChanged,
                    "Set Action Point Location")

CREATE_MERGE_ACTION(ActionPointController, selected_setParameter_merge,
                    ActionPoint, unsigned, parameter, setParameter,
                    signal_dataChanged,
                    "Set Action Point Parameter")

/*
 * ENTITY HITBOX CONTROLLER
 * ========================
 */
template class UnTech::Controller::OrderedListController<EntityHitbox>;

CREATE_SIMPLE_ACTION(EntityHitboxController, selected_setAabb,
                     EntityHitbox, UnTech::ms8rect, aabb, setAabb,
                     signal_dataChanged,
                     "Set Entity Hitbox AABB")

CREATE_SIMPLE_ACTION(EntityHitboxController, selected_setParameter,
                     EntityHitbox, unsigned, parameter, setParameter,
                     signal_dataChanged,
                     "Set Entity Hitbox Parameter")

CREATE_MERGE_ACTION(EntityHitboxController, selected_setAabb_merge,
                    EntityHitbox, UnTech::ms8rect, aabb, setAabb,
                    signal_dataChanged,
                    "Set Entity Hitbox AABB")

CREATE_MERGE_ACTION(EntityHitboxController, selected_setParameter_merge,
                    EntityHitbox, unsigned, parameter, setParameter,
                    signal_dataChanged,
                    "Set Entity Hitbox Parameter")

/*
 * FRAME CONTROLLER
 * ================
 */
template class UnTech::Controller::NamedListController<Frame>;

CREATE_SIMPLE_ACTION(FrameController, selected_setTileHitbox,
                     Frame, UnTech::ms8rect, tileHitbox, setTileHitbox,
                     signal_dataChanged,
                     "Change Frame's Tile Hitbox")

CREATE_SIMPLE_ACTION(FrameController, selected_setSolid,
                     Frame, bool, solid, setSolid,
                     signal_dataChanged,
                     "Change Frame Solidity")

CREATE_MERGE_ACTION(FrameController, selected_setTileHitbox_merge,
                    Frame, UnTech::ms8rect, tileHitbox, setTileHitbox,
                    signal_dataChanged,
                    "Set Frame's Tile Hitbox")

/*
 * PALETTE CONTROLLER
 * ==================
 */
template class UnTech::Controller::OrderedListController<Palette>;

void PaletteController::setSelectedColorId(unsigned colorId)
{
    if (colorId < 16 && (int)colorId != _selectedColorId) {
        _selectedColorId = (int)colorId;
        _signal_selectedColorChanged.emit();
    }
}

void PaletteController::unsetSelectedColor()
{
    if (_selectedColorId >= 0) {
        _selectedColorId = -1;
        _signal_selectedColorChanged.emit();
    }
}

CREATE_INDEXED_ACTION(PaletteController, selected_setColor,
                      Palette, Snes::SnesColor, color,
                      signal_dataChanged,
                      "Change Color")

CREATE_MERGE_INDEXED_ACTION(PaletteController, selected_setColor_merge,
                            Palette, Snes::SnesColor, color,
                            signal_dataChanged,
                            "Change Color")

/*
 * FRAME SET CONTROLLER
 * ====================
 */
namespace UnTech {
namespace MetaSprite {
template <>
sigc::signal<void, const FrameSet*>&
FrameSetController::signal_tilesetChanged<UnTech::Snes::Tileset4bpp8px>()
{
    return _signal_smallTilesetChanged;
}

template <>
sigc::signal<void, const FrameSet*>&
FrameSetController::signal_tilesetChanged<UnTech::Snes::Tileset4bpp16px>()
{
    return _signal_largeTilesetChanged;
}
}
}

template <class TilesetT>
void FrameSetController::selected_tileset_setPixel(const unsigned tileId,
                                                   unsigned x, unsigned y,
                                                   unsigned value)
{
    class Action : public UnTech::Controller::Undo::MergeAction {
    public:
        Action() = delete;
        Action(FrameSet* frameSet,
               const unsigned tileId,
               const typename TilesetT::tile_t& oldTile,
               const typename TilesetT::tile_t& newTile,
               sigc::signal<void, const FrameSet*>& signal)
            : _frameSet(frameSet)
            , _tileId(tileId)
            , _oldTile(oldTile)
            , _newTile(newTile)
            , _signal(signal)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _frameSet->getTileset<TilesetT>().tile(_tileId) = _oldTile;
            _signal.emit(_frameSet);
        }

        virtual void redo() override
        {
            _frameSet->getTileset<TilesetT>().tile(_tileId) = _newTile;
            _signal.emit(_frameSet);
        }

        virtual bool mergeWith(UnTech::Controller::Undo::MergeAction* o) override
        {
            Action* other = dynamic_cast<Action*>(o);

            if (other != nullptr) {
                if (this->_frameSet == other->_frameSet
                    && this->_tileId == other->_tileId) {

                    this->_newTile = other->_newTile;
                    return true;
                }
            }

            return false;
        }

        virtual const std::string& message() const override
        {
            const static std::string message = "Edit Tile";
            return message;
        }

    private:
        FrameSet* _frameSet;
        unsigned _tileId;
        const typename TilesetT::tile_t _oldTile;
        typename TilesetT::tile_t _newTile;
        sigc::signal<void, const FrameSet*>& _signal;
    };

    FrameSet* frameSet = this->selected_editable();
    if (frameSet) {
        TilesetT& tileset = frameSet->getTileset<TilesetT>();

        if (tileId < tileset.size()) {
            typename TilesetT::tile_t oldTile = tileset.tile(tileId);

            tileset.tile(tileId).setPixel(x, y, value);

            typename TilesetT::tile_t newTile = tileset.tile(tileId);

            if (oldTile != newTile) {
                auto& signal = this->signal_tilesetChanged<TilesetT>();

                signal.emit(frameSet);

                baseController().undoStack().add_undoMerge(std::make_unique<Action>(
                    frameSet, tileId, oldTile, newTile, signal));
            }
        }
    }
}

void FrameSetController::selected_smallTileset_setPixel(const unsigned tileId,
                                                        unsigned x, unsigned y,
                                                        unsigned value)
{
    return selected_tileset_setPixel<UnTech::Snes::Tileset4bpp8px>(
        tileId, x, y, value);
}

void FrameSetController::selected_largeTileset_setPixel(const unsigned tileId,
                                                        unsigned x, unsigned y,
                                                        unsigned value)
{
    return selected_tileset_setPixel<UnTech::Snes::Tileset4bpp16px>(
        tileId, x, y, value);
}

void FrameSetController::selected_addTiles(unsigned nNewSmallTiles,
                                           unsigned nNewLargeTiles)
{
    class Action : public UnTech::Controller::Undo::Action {
    public:
        Action() = delete;
        Action(FrameSet* frameSet,
               const unsigned nNewSmallTiles,
               const unsigned nNewLargeTiles,
               FrameSetController& controller)
            : _frameSet(frameSet)
            , _nNewSmallTiles(nNewSmallTiles)
            , _nNewLargeTiles(nNewLargeTiles)
            , _controller(controller)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            // Tiles will always be empty
            for (unsigned t = 0; t < _nNewSmallTiles; t++) {
                _frameSet->smallTileset().removeLastTile();
            }
            for (unsigned t = 0; t < _nNewLargeTiles; t++) {
                _frameSet->largeTileset().removeLastTile();
            }
            emitSignals();
        }

        virtual void redo() override
        {
            for (unsigned t = 0; t < _nNewSmallTiles; t++) {
                _frameSet->smallTileset().addTile();
            }
            for (unsigned t = 0; t < _nNewLargeTiles; t++) {
                _frameSet->largeTileset().addTile();
            }
            emitSignals();
        }

        void emitSignals()
        {
            if (_nNewSmallTiles > 0) {
                _controller.signal_smallTilesetChanged().emit(_frameSet);
            }
            if (_nNewLargeTiles > 0) {
                _controller.signal_largeTilesetChanged().emit(_frameSet);
            }
            _controller.signal_tileCountChanged().emit(_frameSet);
        }

        virtual const std::string& message() const override
        {
            const static std::string message = "Add Tiles";
            return message;
        }

    private:
        FrameSet* _frameSet;
        unsigned _nNewSmallTiles;
        unsigned _nNewLargeTiles;
        FrameSetController& _controller;
    };

    FrameSet* frameSet = this->selected_editable();
    if (frameSet && (nNewSmallTiles > 0 || nNewLargeTiles > 0)) {
        auto a = std::make_unique<Action>(
            frameSet, nNewSmallTiles, nNewLargeTiles, *this);

        a->redo();

        this->baseController().undoStack().add_undo(std::move(a));
    }
}
