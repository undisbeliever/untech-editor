#pragma once

#include "gui/controllers/basecontroller.h"
#include "gui/controllers/helpers/orderedlistcontroller.h"
#include "gui/controllers/helpers/singleitemcontroller.h"
#include "models/metasprite-format/abstractframeset.h"

namespace UnTech {

namespace MetaSprite {
class MetaSpriteController;
}
namespace SpriteImporter {
class SpriteImporterController;
}

namespace MetaSpriteFormat {

class AbstractFrameSetController : public Controller::SingleItemController<AbstractFrameSet> {
    friend class MetaSprite::MetaSpriteController;
    friend class SpriteImporter::SpriteImporterController;

public:
    AbstractFrameSetController(Controller::BaseController& baseController)
        : SingleItemController<AbstractFrameSet>(baseController)
    {
    }
    ~AbstractFrameSetController() = default;

    void selected_setName(const std::string& name);
    void selected_setTilesetType(const TilesetType&);
    void selected_setExportOrderFilename(const std::string& filename);

    auto& signal_nameChanged() { return _signal_nameChanged; }
    auto& signal_exportOrderChanged() { return _signal_exportOrderChanged; }

private:
    sigc::signal<void, const AbstractFrameSet*> _signal_nameChanged;
    sigc::signal<void, const AbstractFrameSet*> _signal_exportOrderChanged;
};
}
}
