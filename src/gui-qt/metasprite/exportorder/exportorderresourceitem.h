/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "models/common/externalfilelist.h"
#include "models/metasprite/frameset-exportorder.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class ExportOrderResourceList;

namespace ExportOrder {
class ExportNameList;
class AlternativesList;
}

class ExportOrderResourceItem : public AbstractExternalResourceItem {
    Q_OBJECT

public:
    using DataT = UnTech::MetaSprite::FrameSetExportOrder;

    using NameReference = UnTech::MetaSprite::NameReference;

public:
    ExportOrderResourceItem(ExportOrderResourceList* parent, size_t index);
    ~ExportOrderResourceItem() = default;

public:
    // may be nullptr
    const DataT* exportOrder() const { return _exportOrders.at(index()); }

    const DataT::ExportName& exportName(bool isFrame, unsigned index);

    ExportOrder::ExportNameList* exportNameList() const { return _exportNameList; }
    ExportOrder::AlternativesList* alternativesList() const { return _alternativesList; }

protected:
    virtual void saveResourceData(const std::string& filename) const final;
    virtual bool loadResourceData(ErrorList& err) final;
    virtual bool compileResource(ErrorList& err) final;

    friend class ExportOrder::ExportNameList;
    friend class ExportOrder::AlternativesList;
    DataT* exportOrderEditable() { return _exportOrders.at(index()); }

private:
    inline auto& exportOrderItem() { return _exportOrders.item(index()); }

private:
    ExternalFileList<DataT>& _exportOrders;

    ExportOrder::ExportNameList* const _exportNameList;
    ExportOrder::AlternativesList* const _alternativesList;
};
}
}
}
