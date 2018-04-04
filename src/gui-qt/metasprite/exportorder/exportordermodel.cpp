/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "exportordermodel.h"
#include "exportorderresourceitem.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite;
using FrameSetExportOrder = UnTech::MetaSprite::FrameSetExportOrder;
using NameReference = UnTech::MetaSprite::NameReference;

const QStringList ExportOrderModel::FLIP_STRINGS({ QString(),
                                                   QString::fromUtf8("hFlip"),
                                                   QString::fromUtf8("vFlip"),
                                                   QString::fromUtf8("hvFlip") });

constexpr quintptr NO_INDEX = ExportOrderModel::InternalIdFormat::NO_INDEX;

constexpr ExportOrderModel::InternalIdFormat invalidInternalId;
static_assert(invalidInternalId.index == NO_INDEX, "bad InvalidInternalId");
static_assert(invalidInternalId.altIndex == NO_INDEX, "bad InvalidInternalId");

ExportOrderModel::ExportOrderModel(QObject* parent)
    : QAbstractItemModel(parent)
    , _exportOrder(nullptr)
{
}

void ExportOrderModel::setExportOrder(ExportOrderResourceItem* exportOrder)
{
    if (_exportOrder != exportOrder) {
        beginResetModel();

        _exportOrder = exportOrder;

        endResetModel();
    }
}

QModelIndex ExportOrderModel::toModelIndex(const InternalIdFormat& internalId) const
{
    if (_exportOrder == nullptr || _exportOrder->exportOrder() == nullptr) {
        return QModelIndex();
    }

    if (internalId.index == NO_INDEX) {
        int row = internalId.isFrame ? FRAMES_ROW : ANIMATIONS_ROW;
        return createIndex(row, 0, internalId.data);
    }
    else {
        const auto* nameList = toExportNameList(internalId);

        if (nameList == nullptr) {
            return QModelIndex();
        }
        if (internalId.index >= nameList->size()) {
            return QModelIndex();
        }

        if (internalId.altIndex == NO_INDEX) {
            return createIndex(internalId.index, 0, internalId.data);
        }
        else {
            const auto& alts = nameList->at(internalId.index).alternatives;

            Q_ASSERT(alts.size() < NO_INDEX);

            if (internalId.altIndex >= alts.size()) {
                return QModelIndex();
            }
            return createIndex(internalId.altIndex, 0, internalId.data);
        }
    }
}

ExportOrderModel::InternalIdFormat ExportOrderModel::toInternalFormat(const QModelIndex& index) const
{
    if (index.isValid()) {
        return index.internalId();
    }
    else {
        return invalidInternalId;
    }
}

const FrameSetExportOrder::ExportName::list_t* ExportOrderModel::toExportNameList(const InternalIdFormat& internalId) const
{
    if (_exportOrder == nullptr) {
        return nullptr;
    }

    const auto* eo = _exportOrder->exportOrder();
    if (eo == nullptr) {
        return nullptr;
    }

    return internalId.isFrame ? &eo->stillFrames : &eo->animations;
}

const FrameSetExportOrder::ExportName* ExportOrderModel::toExportName(const ExportOrderModel::InternalIdFormat& internalId) const
{
    if (_exportOrder == nullptr) {
        return nullptr;
    }

    if (internalId.index == NO_INDEX) {
        return nullptr;
    }

    const auto* nl = toExportNameList(internalId);
    if (nl == nullptr) {
        return nullptr;
    }

    Q_ASSERT(nl->size() < InternalIdFormat::NO_INDEX);

    if (internalId.index >= nl->size()) {
        return nullptr;
    }

    return &nl->at(internalId.index);
}

const NameReference* ExportOrderModel::toAlternative(const ExportOrderModel::InternalIdFormat& internalId) const
{
    if (_exportOrder == nullptr) {
        return nullptr;
    }

    if (internalId.altIndex == NO_INDEX) {
        return nullptr;
    }

    auto en = toExportName(internalId);
    if (en == nullptr) {
        return nullptr;
    }

    Q_ASSERT(en->alternatives.size() < InternalIdFormat::NO_INDEX);

    if (internalId.altIndex >= en->alternatives.size()) {
        return nullptr;
    }

    return &en->alternatives.at(internalId.altIndex);
}

bool ExportOrderModel::checkIndex(const QModelIndex& index) const
{
    if (_exportOrder == nullptr
        || _exportOrder->exportOrder() == nullptr
        || index.isValid() == false
        || index.column() >= N_COLUMNS
        || index.model() != this) {

        return false;
    }

    const InternalIdFormat internalId = index.internalId();
    const unsigned row = index.row();

    if (internalId.index == NO_INDEX) {
        // index = name list node

        return row < N_ROOT_ROWS;
    }
    else if (internalId.altIndex == NO_INDEX) {
        // index = exportName node

        const auto* nameList = toExportNameList(internalId);
        if (nameList == nullptr) {
            return false;
        }
        return internalId.index == row
               && row < nameList->size();
    }
    else {
        // index = alternative node

        const auto* en = toExportName(internalId);
        if (en == nullptr) {
            return false;
        }
        return internalId.altIndex == row
               && row < en->alternatives.size();
    }
}

QModelIndex ExportOrderModel::index(int row, int column, const QModelIndex& parent) const
{
    if (_exportOrder == nullptr
        || _exportOrder->exportOrder() == nullptr
        || row < 0
        || column < 0 || column >= N_COLUMNS) {

        return QModelIndex();
    }

    const InternalIdFormat internalId = parent.internalId();

    if (!parent.isValid()) {
        // parent = root node

        if (row >= N_ROOT_ROWS) {
            return QModelIndex();
        }
        InternalIdFormat id;
        id.isFrame = row == FRAMES_ROW;
        return createIndex(row, column, id.data);
    }
    else if (internalId.index == NO_INDEX) {
        // parent = name list node

        const auto* nameList = toExportNameList(internalId);
        if (nameList == nullptr) {
            return QModelIndex();
        }
        if (unsigned(row) >= nameList->size()) {
            return QModelIndex();
        }

        InternalIdFormat id = internalId;
        id.index = row;
        id.altIndex = NO_INDEX;
        return createIndex(row, column, id.data);
    }
    else if (internalId.altIndex == NO_INDEX) {
        // parent = exportName node

        const auto* en = toExportName(internalId);
        if (en == nullptr) {
            return QModelIndex();
        }
        if (unsigned(row) >= en->alternatives.size()) {
            return QModelIndex();
        }

        InternalIdFormat id = internalId;
        id.altIndex = row;
        return createIndex(row, column, id.data);
    }
    else {
        // parent = alternative node

        return QModelIndex();
    }
}

QModelIndex ExportOrderModel::parent(const QModelIndex& index) const
{
    if (_exportOrder == nullptr
        || _exportOrder->exportOrder() == nullptr
        || index.isValid() == false
        || index.model() != this) {

        return QModelIndex();
    }

    const InternalIdFormat internalId = index.internalId();

    if (internalId.index == NO_INDEX) {
        // index = export list node
        return QModelIndex();
    }
    else if (internalId.altIndex == NO_INDEX) {
        // index = exportName node
        int row = internalId.isFrame ? FRAMES_ROW : ANIMATIONS_ROW;

        InternalIdFormat id = internalId;
        id.index = NO_INDEX;
        return createIndex(row, 0, id.data);
    }
    else {
        // index = alternative node
        const auto* en = toExportName(internalId);

        if (en == nullptr) {
            return QModelIndex();
        }

        InternalIdFormat id = internalId;
        id.altIndex = NO_INDEX;
        return createIndex(internalId.altIndex, 0, id.data);
    }
}

bool ExportOrderModel::hasChildren(const QModelIndex& parent) const
{
    if (_exportOrder == nullptr
        || _exportOrder->exportOrder() == nullptr) {
        return false;
    }

    if (!parent.isValid()) {
        return true;
    }

    const InternalIdFormat internalId = parent.internalId();

    if (internalId.index == NO_INDEX) {
        // parent = name list node
        return true;
    }
    else if (internalId.altIndex == NO_INDEX) {
        // parent = exportName node
        const auto* en = toExportName(internalId);
        return en != nullptr;
    }
    else {
        // parent = alternative node
        return false;
    }
}

int ExportOrderModel::rowCount(const QModelIndex& parent) const
{
    if (_exportOrder == nullptr
        || _exportOrder->exportOrder() == nullptr) {
        return 0;
    }

    if (!parent.isValid()) {
        return N_ROOT_ROWS;
    }

    const InternalIdFormat internalId = parent.internalId();

    if (internalId.index == NO_INDEX) {
        // parent = name list node
        const auto* nameList = toExportNameList(internalId);
        if (nameList == nullptr) {
            return 0;
        }
        return nameList->size();
    }
    else if (internalId.altIndex == NO_INDEX) {
        // parent = exportName node
        const auto* en = toExportName(internalId);
        if (en == nullptr) {
            return 0;
        }
        return en->alternatives.size();
    }
    else {
        // parent = alternative node
        return 0;
    }
}

int ExportOrderModel::columnCount(const QModelIndex&) const
{
    return N_COLUMNS;
}

Qt::ItemFlags ExportOrderModel::flags(const QModelIndex& index) const
{
    if (checkIndex(index) == false
        || _exportOrder->exportOrder() == nullptr) {
        return 0;
    }

    const InternalIdFormat internalId = index.internalId();

    if (internalId.index == NO_INDEX) {
        // index = name list node
        return Qt::ItemIsEnabled;
    }
    else if (internalId.altIndex == NO_INDEX) {
        // index = exportName node
        return Qt::ItemIsEnabled;
    }
    else {
        // index = alternative node
        return Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    }
}

QVariant ExportOrderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= N_COLUMNS
        || orientation != Qt::Horizontal
        || role != Qt::DisplayRole) {

        return QVariant();
    }

    switch (ColumnId(section)) {
    case NAME_COLUMN:
        return tr("Name");

    case FLIP_COLUMN:
        return tr("Flip");
    }

    return QVariant();
}

QVariant ExportOrderModel::data(const QModelIndex& index, int role) const
{
    if (_exportOrder == nullptr
        || _exportOrder->exportOrder() == nullptr
        || index.isValid() == false
        || index.model() != this) {

        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    const InternalIdFormat internalId = index.internalId();
    const unsigned column = index.column();

    if (internalId.index == NO_INDEX) {
        // index = name list node

        if (column == NAME_COLUMN) {
            if (internalId.isFrame) {
                return tr("Frames");
            }
            else {
                return tr("Animations");
            }
        }
    }
    else if (internalId.altIndex == NO_INDEX) {
        // index = exportName node

        const auto* en = toExportName(internalId);
        if (en == nullptr) {
            return QVariant();
        }

        if (column == NAME_COLUMN) {
            return QString::fromStdString(en->name);
        }
    }
    else {
        // index = alternative node

        const auto* alt = toAlternative(internalId);
        if (alt == nullptr) {
            return QVariant();
        }

        if (column == NAME_COLUMN) {
            return QLatin1Literal("ALT: ") + QString::fromStdString(alt->name);
        }
        else {
            int i = (alt->vFlip << 1) | alt->hFlip;
            return FLIP_STRINGS.at(i);
        }
    }

    return QModelIndex();
}
