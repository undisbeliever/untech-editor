/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "exportorderresourceitem.h"
#include "exportorderaccessors.h"
#include "exportorderresourcelist.h"
#include "gui-qt/common/idstringvalidator.h"
#include "models/metatiles/metatiles-serializer.h"

#include <QFileInfo>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite;

using FrameSetExportOrder = UnTech::MetaSprite::FrameSetExportOrder;

ExportOrderResourceItem::ExportOrderResourceItem(ExportOrderResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
    , _exportNameList(new ExportOrder::ExportNameList(this))
    , _alternativesList(new ExportOrder::AlternativesList(this))
{
    Q_ASSERT(index < exportOrderList().size());

    setFilename(QString::fromStdString(exportOrderItem().filename));
}

void ExportOrderResourceItem::saveResourceData(const std::string& filename) const
{
    using namespace UnTech::MetaSprite;

    auto* eo = this->exportOrder();

    if (eo) {
        saveFrameSetExportOrder(*eo, filename);
    }
}

bool ExportOrderResourceItem::loadResourceData(RES::ErrorList& err)
{
    using namespace UnTech::MetaSprite;

    auto& eoItem = exportOrderItem();

    if (eoItem.filename.empty()) {
        err.addError("Missing filename");
        return false;
    }

    try {
        eoItem.value = loadFrameSetExportOrder(eoItem.filename);
        setName(QString::fromStdString(eoItem.value->name));
        return true;
    }
    catch (const std::exception& ex) {
        eoItem.value = nullptr;

        err.addError(ex.what());
        return false;
    }
}

bool ExportOrderResourceItem::compileResource(RES::ErrorList& err)
{
    const auto* eo = exportOrder();

    if (eo == nullptr) {
        err.addError("Unable to load file");
        return false;
    }

    return eo->validate(err);
}