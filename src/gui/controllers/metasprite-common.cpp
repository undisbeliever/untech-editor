#include "metasprite-common.h"
#include "gui/controllers/helpers/actionhelper.h"
#include "models/metasprite-common/abstractframeset.h"

using namespace UnTech::MetaSpriteCommon;

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
