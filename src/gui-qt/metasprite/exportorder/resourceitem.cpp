/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "resourcelist.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/common/idstringvalidator.h"
#include "models/metatiles/metatiles-serializer.h"

#include <QFileInfo>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::ExportOrder;

using FrameSetExportOrder = UnTech::MetaSprite::FrameSetExportOrder;

ResourceItem::ResourceItem(ResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
    , _exportOrders(parent->exportOrders())
    , _exportNameList(new ExportOrder::ExportNameList(this))
    , _alternativesList(new ExportOrder::AlternativesList(this))
{
    Q_ASSERT(index < _exportOrders.size());

    setFilename(QString::fromStdString(exportOrderItem().filename));

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

bool ResourceItem::editExportOrder_setName(const UnTech::idstring& name)
{
    if (name.isValid() == false) {
        return false;
    }
    return UndoHelper(this).editName(name);
}

void ResourceItem::saveResourceData(const std::string& filename) const
{
    using namespace UnTech::MetaSprite;

    auto* eo = this->exportOrder();

    if (eo) {
        saveFrameSetExportOrder(*eo, filename);
    }
}

bool ResourceItem::loadResourceData(ErrorList& err)
{
    using namespace UnTech::MetaSprite;

    auto& eoItem = exportOrderItem();

    setFilename(QString::fromStdString(eoItem.filename));

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

bool ResourceItem::compileResource(ErrorList& err)
{
    const auto* eo = exportOrder();

    if (eo == nullptr) {
        err.addError("Unable to load file");
        return false;
    }

    return eo->validate(err);
}
