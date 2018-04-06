/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "exportordermodel.h"
#include "exportorderresourceitem.h"

#include "exportorderlists.h"
#include "gui-qt/undo/listundohelper.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::ExportOrder;

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
    : AbstractPropertyModel(parent)
    , _exportOrder(nullptr)
    , _properties({
          Property(tr("Name"), 0, PropertyType::IDSTRING),
          Property(tr("Flip"), 1, PropertyType::COMBO, FLIP_STRINGS, QVariantList{ 0, 1, 2, 3 }),
      })
{
}

void ExportOrderModel::setExportOrder(ExportOrderResourceItem* exportOrder)
{
    if (_exportOrder != exportOrder) {
        if (_exportOrder) {
            _exportOrder->exportNameList()->disconnect(this);
            _exportOrder->alternativesList()->disconnect(this);
        }

        beginResetModel();

        _exportOrder = exportOrder;

        endResetModel();

        if (_exportOrder) {
            connect(_exportOrder->exportNameList(), &ExportNameList::dataChanged,
                    this, &ExportOrderModel::onExportNameChanged);
            connect(_exportOrder->alternativesList(), &AlternativesList::dataChanged,
                    this, &ExportOrderModel::onExportNameAltChanged);
        }
    }
}

void ExportOrderModel::onExportNameChanged(bool isFrame, unsigned index)
{
    InternalIdFormat internalId;
    internalId.isFrame = isFrame;
    internalId.index = index;

    auto* en = toExportName(internalId);
    if (en) {
        QModelIndex mIndex = toModelIndex(internalId);
        auto mIndex2 = createIndex(mIndex.row(), N_COLUMNS, mIndex.internalId());

        emit dataChanged(mIndex, mIndex2,
                         { Qt::DisplayRole, Qt::EditRole });
    }
}

void ExportOrderModel::onExportNameAltChanged(bool isFrame, unsigned index, unsigned altIndex)
{
    InternalIdFormat internalId;
    internalId.isFrame = isFrame;
    internalId.index = index;
    internalId.altIndex = altIndex;

    auto* en = toExportName(internalId);
    if (en && altIndex < en->alternatives.size()) {
        QModelIndex mIndex = toModelIndex(internalId);
        auto mIndex2 = createIndex(mIndex.row(), N_COLUMNS, mIndex.internalId());

        emit dataChanged(mIndex, mIndex2,
                         { Qt::DisplayRole, Qt::EditRole });
    }
}

const Property& ExportOrderModel::propertyForIndex(const QModelIndex& index) const
{
    const InternalIdFormat internalId = index.internalId();

    Q_ASSUME(_properties.size() == N_COLUMNS);

    if (!index.isValid()) {
        return blankProperty;
    }
    else if (internalId.index == NO_INDEX) {
        return blankProperty;
    }
    else if (internalId.altIndex == NO_INDEX) {
        if (index.column() == NAME_COLUMN) {
            return _properties.at(0);
        }
        else {
            return blankProperty;
        }
    }
    else {
        if (index.column() == NAME_COLUMN) {
            return _properties.at(0);
        }
        else {
            return _properties.at(1);
        }
    }
}

bool ExportOrderModel::isListItem(const QModelIndex&) const
{
    return false;
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

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    const InternalIdFormat internalId = index.internalId();

    if (internalId.altIndex == NO_INDEX) {
        // index = exportName node

        if (index.column() == NAME_COLUMN) {
            flags |= Qt::ItemIsEditable;
        }
    }
    else {
        // index = alternative node

        flags |= Qt::ItemIsEditable | Qt::ItemNeverHasChildren;
    }

    return flags;
}

QVariant ExportOrderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= N_COLUMNS
        || orientation != Qt::Horizontal
        || role != Qt::DisplayRole) {

        return QVariant();
    }

    return _properties.at(section).title;
}

QVariant ExportOrderModel::data(const QModelIndex& index, int role) const
{
    if (_exportOrder == nullptr
        || _exportOrder->exportOrder() == nullptr
        || index.isValid() == false
        || index.model() != this) {

        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    const InternalIdFormat internalId = index.internalId();
    const unsigned column = index.column();

    if (internalId.index == NO_INDEX) {
        // index = name list node

        if (role == Qt::DisplayRole && column == NAME_COLUMN) {
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
            if (role == Qt::DisplayRole) {
                return QLatin1String("ALT: ") + QString::fromStdString(alt->name);
            }
            else {
                return QString::fromStdString(alt->name);
            }
        }
        else {
            int i = (alt->vFlip << 1) | alt->hFlip;

            if (role == Qt::DisplayRole) {
                return FLIP_STRINGS.at(i);
            }
            else {
                return i;
            }
        }
    }

    return QModelIndex();
}

bool ExportOrderModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (checkIndex(index) == false
        || role != Qt::EditRole) {

        return false;
    }

    using ExportName = FrameSetExportOrder::ExportName;

    const InternalIdFormat internalId = index.internalId();
    const unsigned column = index.column();

    if (internalId.altIndex == NO_INDEX) {
        // index = exportName node

        if (column == FLIP_COLUMN) {
            return false;
        }

        idstring name = value.toString().toStdString();
        if (name.isValid() == false) {
            return false;
        }
        ExportNameUndoHelper undoHelper(_exportOrder->exportNameList(), internalId.isFrame);
        undoHelper.editField<idstring>(internalId.index, name,
                                       tr("Edit Export Name"),
                                       [](ExportName& en) -> idstring& { return en.name; });
    }
    else {
        // index = alternative node

        const NameReference* oldAltPtr = toAlternative(internalId);
        Q_ASSERT(oldAltPtr);
        const NameReference& oldAlt = *oldAltPtr;
        NameReference newAlt = oldAlt;

        if (column == NAME_COLUMN) {
            newAlt.name = value.toString().toStdString();

            if (newAlt.name.isValid() == false) {
                return false;
            }
        }
        else {
            newAlt.hFlip = value.toUInt() & 1;
            newAlt.vFlip = value.toUInt() & 2;
        }

        if (newAlt != oldAlt) {
            AlternativesUndoHelper undoHelper(_exportOrder->alternativesList(), internalId.isFrame, internalId.index);
            undoHelper.edit(internalId.altIndex, newAlt);
        }
    }

    return false;
}
