#include "sprite-importer.h"
#include "gui/controllers/helpers/actionhelper.h"
#include "gui/controllers/helpers/documentcontroller.hpp"
#include "gui/controllers/helpers/mergeactionhelper.h"
#include "gui/controllers/helpers/namedlistcontroller.hpp"
#include "gui/controllers/helpers/orderedlistcontroller.hpp"
#include "gui/controllers/helpers/spriteselectedtypecontroller.hpp"
#include "models/sprite-importer.h"

using namespace UnTech::SpriteImporter;

template class UnTech::Controller::SpriteSelectedTypeController<SpriteImporterController>;

/*
 * SPRITE IMPORTER CONTROLLER
 * ==========================
 */
template class UnTech::Controller::DocumentController<SpriteImporterDocument>;

SpriteImporterController::SpriteImporterController(std::unique_ptr<Controller::ControllerInterface> interface)
    : DocumentController(std::move(interface))
    , _frameSetController(*this)
    , _abstractFrameSetController(*this)
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
        }
        else {
            _frameController.setList(nullptr);
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
}

void SpriteImporterController::emitAllDataChanged()
{
    _abstractFrameSetController.emitAllDataChanged();

    _frameSetController.signal_selectedChanged().emit();
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
                     FrameObject, UnTech::upoint, location, setLocation,
                     signal_dataChanged,
                     "Set Frame Object Location")

CREATE_DUAL_ACTION(FrameObjectController, selected_setLocationAndSize,
                   FrameObject,
                   UnTech::upoint, location, setLocation,
                   FrameObject::ObjectSize, size, setSize,
                   signal_dataChanged,
                   "Set Frame Object Location")

CREATE_MERGE_ACTION(FrameObjectController, selected_setLocation_merge,
                    FrameObject, UnTech::upoint, location, setLocation,
                    signal_dataChanged,
                    "Set Frame Object Location")

/*
 * ACTION POINT CONTROLLER
 * =======================
 */
template class UnTech::Controller::OrderedListController<ActionPoint>;

CREATE_SIMPLE_ACTION(ActionPointController, selected_setLocation,
                     ActionPoint, UnTech::upoint, location, setLocation,
                     signal_dataChanged,
                     "Set Action Point Location")

CREATE_SIMPLE_ACTION(ActionPointController, selected_setParameter,
                     ActionPoint, unsigned, parameter, setParameter,
                     signal_dataChanged,
                     "Set Action Point Parameter")

CREATE_MERGE_ACTION(ActionPointController, selected_setLocation_merge,
                    ActionPoint, UnTech::upoint, location, setLocation,
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
                     EntityHitbox, UnTech::urect, aabb, setAabb,
                     signal_dataChanged,
                     "Set Entity Hitbox AABB")

CREATE_SIMPLE_ACTION(EntityHitboxController, selected_setParameter,
                     EntityHitbox, unsigned, parameter, setParameter,
                     signal_dataChanged,
                     "Set Entity Hitbox Parameter")

CREATE_MERGE_ACTION(EntityHitboxController, selected_setAabb_merge,
                    EntityHitbox, UnTech::urect, aabb, setAabb,
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

CREATE_SIMPLE_ACTION2(FrameController, selected_setLocation,
                      Frame, UnTech::urect, location, setLocation,
                      signal_dataChanged, signal_frameSizeChanged,
                      "Change Frame Location")

CREATE_SIMPLE_ACTION2(FrameController, selected_setGridLocation,
                      Frame, UnTech::upoint, gridLocation, setGridLocation,
                      signal_dataChanged, signal_frameSizeChanged,
                      "Change Frame Grid Location")

CREATE_SIMPLE_ACTION2(FrameController, selected_setUseGridLocation,
                      Frame, bool, useGridLocation, setUseGridLocation,
                      signal_dataChanged, signal_frameSizeChanged,
                      "Change Use Frame Grid Location")

CREATE_SIMPLE_ACTION(FrameController, selected_setOrigin,
                     Frame, UnTech::upoint, origin, setOrigin,
                     signal_dataChanged,
                     "Change Frame Origin")

CREATE_SIMPLE_ACTION(FrameController, selected_setUseGridOrigin,
                     Frame, bool, useGridOrigin, setUseGridOrigin,
                     signal_dataChanged,
                     "Change Use Frame Grid Origin")

CREATE_SIMPLE_ACTION(FrameController, selected_setSolid,
                     Frame, bool, solid, setSolid,
                     signal_dataChanged,
                     "Change Frame Solidity")

CREATE_SIMPLE_ACTION(FrameController, selected_setTileHitbox,
                     Frame, UnTech::urect, tileHitbox, setTileHitbox,
                     signal_dataChanged,
                     "Change Frame's Tile Hitbox")

CREATE_SIMPLE_ACTION(FrameController, selected_setSpriteOrder,
                     Frame, unsigned, spriteOrder, setSpriteOrder,
                     signal_dataChanged,
                     "Change Frame Order")

CREATE_MERGE_ACTION2(FrameController, selected_setLocation_merge,
                     Frame, UnTech::urect, location, setLocation,
                     signal_dataChanged, signal_frameSizeChanged,
                     "Change Frame Location")

CREATE_MERGE_ACTION2(FrameController, selected_setGridLocation_merge,
                     Frame, UnTech::upoint, gridLocation, setGridLocation,
                     signal_dataChanged, signal_frameSizeChanged,
                     "Change Frame Grid Location")

CREATE_MERGE_ACTION(FrameController, selected_setOrigin_merge,
                    Frame, UnTech::upoint, origin, setOrigin,
                    signal_dataChanged,
                    "Change Frame Origin")

CREATE_MERGE_ACTION(FrameController, selected_setTileHitbox_merge,
                    Frame, UnTech::urect, tileHitbox, setTileHitbox,
                    signal_dataChanged,
                    "Change Frame's Tile Hitbox")

/*
 * FRAME SET CONTROLLER
 * ====================
 */

void FrameSetController::setSelectTransparentMode(bool mode)
{
    if (_selectTransparentMode != mode) {
        _selectTransparentMode = mode;
        _signal_selectTransparentModeChanged.emit();
    }
}

CREATE_SIMPLE_ACTION(FrameSetController, selected_setTransparentColor,
                     FrameSet, UnTech::rgba, transparentColor, setTransparentColor,
                     signal_dataChanged,
                     "Change Image")

CREATE_SIMPLE_ACTION2(FrameSetController, selected_setImageFilename,
                      FrameSet, std::string, imageFilename, setImageFilename,
                      signal_dataChanged, signal_imageChanged,
                      "Change Image")

CREATE_PARAMETER_ACTION2(FrameSetController, selected_setGridFrameSize,
                         FrameSet, grid, UnTech::usize, frameSize, setFrameSize,
                         signal_dataChanged, signal_gridChanged,
                         "Change Grid Frame Size")

CREATE_PARAMETER_ACTION2(FrameSetController, selected_setGridOffset,
                         FrameSet, grid, UnTech::upoint, offset, setOffset,
                         signal_dataChanged, signal_gridChanged,
                         "Change Grid Offset")

CREATE_PARAMETER_ACTION2(FrameSetController, selected_setGridPadding,
                         FrameSet, grid, UnTech::usize, padding, setPadding,
                         signal_dataChanged, signal_gridChanged,
                         "Change Grid Padding")

CREATE_PARAMETER_ACTION2(FrameSetController, selected_setGridOrigin,
                         FrameSet, grid, UnTech::upoint, origin, setOrigin,
                         signal_dataChanged, signal_gridChanged,
                         "Change Grid Origin")

CREATE_MERGE_PARAM_ACTION2(FrameSetController, selected_setGridFrameSize_merge,
                           FrameSet, grid, UnTech::usize, frameSize, setFrameSize,
                           signal_dataChanged, signal_gridChanged,
                           "Change Grid Frame Size")

CREATE_MERGE_PARAM_ACTION2(FrameSetController, selected_setGridOffset_merge,
                           FrameSet, grid, UnTech::upoint, offset, setOffset,
                           signal_dataChanged, signal_gridChanged,
                           "Change Grid Offset")

CREATE_MERGE_PARAM_ACTION2(FrameSetController, selected_setGridPadding_merge,
                           FrameSet, grid, UnTech::usize, padding, setPadding,
                           signal_dataChanged, signal_gridChanged,
                           "Change Grid Padding")

CREATE_MERGE_PARAM_ACTION2(FrameSetController, selected_setGridOrigin_merge,
                           FrameSet, grid, UnTech::upoint, origin, setOrigin,
                           signal_dataChanged, signal_gridChanged,
                           "Change Grid Origin")
