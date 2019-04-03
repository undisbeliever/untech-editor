/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/abstractpropertymodel.h"
#include "models/metasprite/frameset-exportorder.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ExportOrder {
class ResourceItem;

class ExportOrderModel : public AbstractPropertyModel {
    Q_OBJECT

    using FrameSetExportOrder = UnTech::MetaSprite::FrameSetExportOrder;
    using NameReference = UnTech::MetaSprite::NameReference;

public:
    static const QStringList FLIP_STRINGS;

    enum ColumnId {
        NAME_COLUMN,
        FLIP_COLUMN,
    };
    constexpr static int N_COLUMNS = 2;

    enum RootRowId {
        FRAMES_ROW,
        ANIMATIONS_ROW,
    };
    constexpr static int N_ROOT_ROWS = 2;

    union InternalIdFormat {
        constexpr static int INDEX_BITS = sizeof(quintptr) > 4 ? 31 : 15;
        constexpr static quintptr NO_INDEX = (1U << INDEX_BITS) - 1;

        quintptr data = ~(quintptr(0));
        struct {
            quintptr isFrame : 1;
            quintptr index : INDEX_BITS;
            quintptr altIndex : INDEX_BITS;
        };

        constexpr explicit InternalIdFormat()
            : isFrame(1)
            , index(NO_INDEX)
            , altIndex(NO_INDEX)
        {
        }

        constexpr InternalIdFormat(quintptr d)
            : data(d)
        {
        }
    };
    static_assert(sizeof(InternalIdFormat) == sizeof(quintptr),
                  "Invalid InternalIdFormat size.");

public:
    explicit ExportOrderModel(QObject* parent = nullptr);
    ~ExportOrderModel() = default;

    void setExportOrder(ResourceItem* exportOrder);

    virtual const Property& propertyForIndex(const QModelIndex& index) const final;
    virtual bool isListItem(const QModelIndex& index) const final;

    QModelIndex toModelIndex(const InternalIdFormat& index) const;
    InternalIdFormat toInternalFormat(const QModelIndex& index) const;

    const std::vector<FrameSetExportOrder::ExportName>* toExportNameList(const InternalIdFormat& internalId) const;
    const FrameSetExportOrder::ExportName* toExportName(const InternalIdFormat& internalId) const;
    const NameReference* toAlternative(const ExportOrderModel::InternalIdFormat& internalId) const;

    bool checkIndex(const QModelIndex& index) const;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const final;
    virtual QModelIndex parent(const QModelIndex& index) const final;

    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const final;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex& index, int role) const final;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) final;

private slots:
    void onListChanged();
    void onExportNameChanged(bool isFrame, unsigned index);
    void onExportNameAltChanged(bool isFrame, unsigned index, unsigned altIndex);

private:
    ResourceItem* _exportOrder;
    const QVector<Property> _properties;
};
}
}
}
}
