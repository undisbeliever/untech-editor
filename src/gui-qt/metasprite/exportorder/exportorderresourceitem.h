/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/metasprite/metaspriteproject.h"
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

    MetaSpriteProject* project() const { return static_cast<MetaSpriteProject*>(_project); }

public:
    // may be nullptr
    const DataT* exportOrder() const
    {
        return project()->metaSpriteProject()->frameSetExportOrders.at(index());
    }

    const DataT::ExportName& exportName(bool isFrame, unsigned index);

    ExportOrder::ExportNameList* exportNameList() const { return _exportNameList; }
    ExportOrder::AlternativesList* alternativesList() const { return _alternativesList; }

protected:
    virtual void saveResourceData(const std::string& filename) const final;
    virtual bool loadResourceData(ErrorList& err) final;
    virtual bool compileResource(ErrorList& err) final;

    friend class ExportOrder::ExportNameList;
    friend class ExportOrder::AlternativesList;
    DataT* exportOrderEditable()
    {
        return project()->metaSpriteProject()->frameSetExportOrders.at(index());
    }

private:
    inline const auto& exportOrderList() const
    {
        return project()->metaSpriteProject()->frameSetExportOrders;
    }
    inline auto& exportOrderItem()
    {
        return project()->metaSpriteProject()->frameSetExportOrders.item(index());
    }

private:
    ExportOrder::ExportNameList* const _exportNameList;
    ExportOrder::AlternativesList* const _alternativesList;
};
}
}
}
