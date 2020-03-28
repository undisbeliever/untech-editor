/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "models/common/externalfilelist.h"
#include "models/metasprite/frameset-exportorder.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ExportOrder {
class ResourceList;
class ExportNameList;
class AlternativesList;

class ResourceItem : public AbstractExternalResourceItem {
    Q_OBJECT

public:
    using DataT = UnTech::MetaSprite::FrameSetExportOrder;
    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

    using NameReference = UnTech::MetaSprite::NameReference;

public:
    ResourceItem(ResourceList* parent, size_t index);
    ~ResourceItem() = default;

    static QString typeName() { return tr("Export Order"); }

    // may be nullptr
    const DataT* exportOrder() const { return _exportOrders.at(index()); }

    const DataT::ExportName& exportName(bool isFrame, unsigned index);

    ExportOrder::ExportNameList* exportNameList() const { return _exportNameList; }
    ExportOrder::AlternativesList* alternativesList() const { return _alternativesList; }

    bool editExportOrder_setName(const idstring& name);

protected:
    virtual void saveResourceData(const std::filesystem::path& filename) const final;
    virtual bool loadResourceData(ErrorList& err) final;
    virtual bool compileResource(ErrorList& err) final;

    friend class ExportNameList;
    friend class AlternativesList;
    friend class Accessor::ResourceItemUndoHelper<ResourceItem>;
    const DataT* data() const { return _exportOrders.at(index()); }
    DataT* dataEditable() { return _exportOrders.at(index()); }

private:
    inline auto& exportOrderItem() { return _exportOrders.item(index()); }

private:
    ExternalFileList<DataT>& _exportOrders;

    ExportNameList* const _exportNameList;
    AlternativesList* const _alternativesList;
};
}
}
}
}
