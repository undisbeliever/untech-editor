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

namespace RES = UnTech::Resources;

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
        return project()->metaSpriteProject()->exportOrders.at(index());
    }

    const DataT::ExportName& exportName(bool isFrame, unsigned index);

protected:
    friend class EditExportOrderExportNameCommand;
    void setExportName(bool isFrame, unsigned index, const idstring& name);

    friend class EditExportOrderAlternativeCommand;
    void setExportNameAlternative(bool isFrame, unsigned index, unsigned altIndex,
                                  const NameReference& alt);

protected:
    virtual void saveResourceData(const std::string& filename) const final;
    virtual bool loadResourceData(RES::ErrorList& err) final;
    virtual bool compileResource(RES::ErrorList& err) final;

signals:
    void exportNameChanged(bool isFrame, unsigned index);
    void exportNameAltChanged(bool isFrame, unsigned index, unsigned altIndex);

private:
    inline const auto& exportOrderList() const
    {
        return project()->metaSpriteProject()->exportOrders;
    }

    inline auto& exportOrderItem()
    {
        return project()->metaSpriteProject()->exportOrders.item(index());
    }
};
}
}
}
