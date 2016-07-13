#pragma once

#include "gui/controllers/basecontroller.h"
#include "gui/controllers/helpers/namedlistcontroller.h"
#include "gui/controllers/helpers/orderedlistcontroller.h"
#include "gui/controllers/helpers/singleitemcontroller.h"
#include "models/metasprite-common/abstractframeset.h"

namespace UnTech {

namespace MetaSprite {
class MetaSpriteController;
}
namespace SpriteImporter {
class SpriteImporterController;
}

class AbstractFrameSetController;

namespace MetaSpriteCommon {

class AnimationInstructionController : public Controller::OrderedListController<AnimationInstruction> {
public:
    AnimationInstructionController(Controller::BaseController&);
    ~AnimationInstructionController() = default;

    void selected_setOperation(const AnimationBytecode&);
    void selected_setParameter(const int&);
    void selected_setFrame(const FrameReference&);
    void selected_setGotoLabel(const std::string&);
};

class AnimationController : public Controller::NamedListController<Animation> {
    friend class AbstractFrameSetController;

public:
    AnimationController(Controller::BaseController&);
    ~AnimationController() = default;
};

class AbstractFrameSetController : public Controller::SingleItemController<AbstractFrameSet> {
    friend class MetaSprite::MetaSpriteController;
    friend class SpriteImporter::SpriteImporterController;

public:
    AbstractFrameSetController(Controller::BaseController&);
    ~AbstractFrameSetController() = default;

    auto& animationController() { return _animationController; }
    auto& animationInstructionController() { return _animationInstructionController; }

    // emits `signal_listChanged` and `signal_selectedChanged` for
    // all child controllers.
    void emitAllDataChanged();

    void selected_setName(const std::string& name);
    void selected_setTilesetType(const TilesetType&);
    void selected_setExportOrderFilename(const std::string& filename);

    auto& signal_nameChanged() { return _signal_nameChanged; }
    auto& signal_exportOrderChanged() { return _signal_exportOrderChanged; }

private:
    sigc::signal<void, const AbstractFrameSet*> _signal_nameChanged;
    sigc::signal<void, const AbstractFrameSet*> _signal_exportOrderChanged;

    AnimationController _animationController;
    AnimationInstructionController _animationInstructionController;
};
}
}
