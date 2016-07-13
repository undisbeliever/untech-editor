#include "metasprite-common.h"
#include "gui/controllers/helpers/actionhelper.h"
#include "gui/controllers/helpers/namedlistcontroller.hpp"
#include "gui/controllers/helpers/orderedlistcontroller.hpp"
#include "models/metasprite-common/abstractframeset.h"

using namespace UnTech::MetaSpriteCommon;

/*
 * ABSTRACT FRAMESET CONTROLLER
 * ============================
 */

AbstractFrameSetController::AbstractFrameSetController(Controller::BaseController& baseController)
    : SingleItemController<AbstractFrameSet>(baseController)
    , _animationController(baseController)
    , _animationInstructionController(baseController)
{

    /*
     * SIGNALS
     * -------
     */
    signal_selectedChanged().connect([this](void) {
        auto* frameSet = selected_editable();
        if (frameSet != nullptr) {
            _animationController.setList(&frameSet->animations());
        }
        else {
            _animationController.setList(nullptr);
        }
    });

    _animationController.signal_selectedChanged().connect([this](void) {
        auto* animation = _animationController.selected_editable();
        if (animation != nullptr) {
            _animationInstructionController.setList(&animation->instructions());
        }
        else {
            _animationInstructionController.setList(nullptr);
        }
    });
}

void AbstractFrameSetController::emitAllDataChanged()
{
    signal_dataChanged().emit(selected());
    _animationController.signal_selectedChanged().emit();
    _animationController.signal_listChanged().emit();
    _animationInstructionController.signal_selectedChanged().emit();
    _animationInstructionController.signal_listChanged().emit();
}

CREATE_SIMPLE_ACTION2(AbstractFrameSetController, selected_setName,
                      AbstractFrameSet,
                      std::string, name, setName,
                      signal_dataChanged, signal_nameChanged,
                      "Change Name")

CREATE_SIMPLE_ACTION(AbstractFrameSetController, selected_setTilesetType,
                     AbstractFrameSet,
                     TilesetType, tilesetType, setTilesetType,
                     signal_dataChanged,
                     "Change Name")

CREATE_HANDLED_ACTION2(AbstractFrameSetController, selected_setExportOrderFilename,
                       AbstractFrameSet,
                       std::string, exportOrderFilename, loadExportOrderDocument,
                       signal_dataChanged, signal_exportOrderChanged,
                       "Change Export Order Document",
                       "Unable to load export order document")

/*
 * ANIMATION INSTRUCTION CONTROLLER
 * ================================
 */
template class UnTech::Controller::OrderedListController<AnimationInstruction>;

AnimationInstructionController::AnimationInstructionController(Controller::BaseController& baseController)
    : OrderedListController(baseController)
{
}

CREATE_SIMPLE_ACTION(AnimationInstructionController, selected_setOperation,
                     AnimationInstruction,
                     AnimationBytecode, operation, setOperation,
                     signal_dataChanged,
                     "Change Animation Operation")

CREATE_SIMPLE_ACTION(AnimationInstructionController, selected_setParameter,
                     AnimationInstruction,
                     int, parameter, setParameter,
                     signal_dataChanged,
                     "Change Animation Parameter")

CREATE_SIMPLE_ACTION(AnimationInstructionController, selected_setFrame,
                     AnimationInstruction,
                     FrameReference, frame, setFrame,
                     signal_dataChanged,
                     "Change Animation Frame")

CREATE_SIMPLE_ACTION(AnimationInstructionController, selected_setGotoLabel,
                     AnimationInstruction,
                     std::string, gotoLabel, setGotoLabel,
                     signal_dataChanged,
                     "Change Animation Goto")

/*
 * ANIMATION CONTROLLER
 * ====================
 */
template class UnTech::Controller::NamedListController<Animation>;

AnimationController::AnimationController(Controller::BaseController& baseController)
    : NamedListController(baseController)
{
}
