/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "structfieldsmodel.h"
#include "accessors.h"
#include "resourceitem.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Entity::EntityRomStructs;

static const QStringList TYPE_STRINGS = {
    "uint8",
    "uint16",
    "uint24",
    "uint32",
    "sint8",
    "sint16",
    "sint24",
    "sint32",
};
static const QVariantList TYPE_VARS = {
    int(EN::DataType::UINT8),
    int(EN::DataType::UINT16),
    int(EN::DataType::UINT24),
    int(EN::DataType::UINT32),
    int(EN::DataType::SINT8),
    int(EN::DataType::SINT16),
    int(EN::DataType::SINT24),
    int(EN::DataType::SINT32),
};

StructFieldsModel::StructFieldsModel(QObject* parent)
    : AbstractPropertyModel(parent)
    , _properties({
          Property(tr("Name"), 0, PropertyType::IDSTRING),
          Property(tr("Type"), 1, PropertyType::COMBO, TYPE_STRINGS, TYPE_VARS),
          Property(tr("Default Value"), 2, PropertyType::STRING),
          Property(tr("Comment"), 3, PropertyType::STRING),
      })
    , _item(nullptr)
    , _data()
{
}

void StructFieldsModel::setResourceItem(ResourceItem* item)
{
    if (_item == item) {
        return;
    }

    if (_item) {
        _item->disconnect(this);
        _item->structList()->disconnect(this);
    }
    _item = item;

    if (item) {
        _data.setEntityRomData(&item->project()->projectFile()->entityRomData);

        connect(item->structList(), &EntityRomStructList::selectedIndexChanged,
                this, &StructFieldsModel::rebuildData);

        connect(item->structList(), &EntityRomStructList::nameChanged,
                this, &StructFieldsModel::rebuildData);
        connect(item->structList(), &EntityRomStructList::parentChanged,
                this, &StructFieldsModel::rebuildData);

        connect(item->structFieldList(), &EntityRomStructFieldList::listChanged,
                this, &StructFieldsModel::rebuildData);

        connect(item->structFieldList(), &EntityRomStructFieldList::dataChanged,
                this, &StructFieldsModel::onStructFieldDataChanged);
    }

    rebuildData();
}

void StructFieldsModel::rebuildData()
{
    emit layoutAboutToBeChanged();

    if (_item) {
        _data.rebuildDataForStruct(_item->structList()->selectedItem());
    }
    else {
        _data.clear();
    }

    emit layoutChanged();
}

void StructFieldsModel::onStructFieldDataChanged(size_t structId, size_t index)
{
    if (structId != _item->structList()->selectedIndex()) {
        rebuildData();
    }
    else {
        auto* selectedStruct = _item->structList()->selectedItem();
        Q_ASSERT(selectedStruct);
        Q_ASSERT(index < selectedStruct->fields.size());

        int row = _data.updateFieldInChildStruct(index, selectedStruct->fields.at(index));
        emit dataChanged(createIndex(row, 0), createIndex(row, N_COLUMNS - 1));
    }
}

const Property& StructFieldsModel::propertyForIndex(const QModelIndex& index) const
{
    Q_ASSUME(_properties.size() == N_COLUMNS);

    if (!index.isValid()
        || index.column() < 0 || index.column() >= N_COLUMNS) {

        return blankProperty;
    }

    return _properties.at(index.column());
}

bool StructFieldsModel::isListItem(const QModelIndex&) const
{
    return false;
}

bool StructFieldsModel::checkIndex(const QModelIndex& index) const
{
    if (_item == nullptr
        || index.isValid() == false
        || index.column() >= N_COLUMNS
        || index.model() != this
        || index.row() >= _data.size()) {

        return false;
    }

    return index.row() < _data.size();
}

QModelIndex StructFieldsModel::toModelIndex(size_t fieldIndex) const
{
    int row = int(fieldIndex) + _data.fieldsInParent();
    if (row < 0 || row >= _data.size()) {
        return QModelIndex();
    }
    return createIndex(row, 0);
}

size_t StructFieldsModel::toFieldIndex(const QModelIndex& index) const
{
    if (index.isValid() == false) {
        return INT_MAX;
    }
    int fi = index.row() - _data.fieldsInParent();

    if (fi < 0) {
        return INT_MAX;
    }
    return size_t(fi);
}

QModelIndex StructFieldsModel::index(int row, int column, const QModelIndex& parent) const
{
    if (_item == nullptr
        || parent.isValid()
        || row < 0 || row >= _data.size()
        || column < 0 || column >= N_COLUMNS) {

        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex StructFieldsModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

bool StructFieldsModel::hasChildren(const QModelIndex& parent) const
{
    if (_item == nullptr) {
        return false;
    }

    return !parent.isValid();
}

int StructFieldsModel::rowCount(const QModelIndex& parent) const
{
    if (_item == nullptr
        || parent.isValid()) {

        return 0;
    }

    return _data.size();
}

int StructFieldsModel::columnCount(const QModelIndex&) const
{
    return N_COLUMNS;
}

Qt::ItemFlags StructFieldsModel::flags(const QModelIndex& index) const
{
    if (checkIndex(index) == false) {
        return Qt::ItemFlags{};
    }

    const auto& field = _data.at(index.row());
    if (field.isParent) {
        return Qt::ItemIsSelectable;
    }
    else {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
}

QVariant StructFieldsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= N_COLUMNS
        || orientation != Qt::Horizontal
        || role != Qt::DisplayRole) {

        return QVariant();
    }
    return _properties.at(section).title;
}

QVariant StructFieldsModel::data(const QModelIndex& index, int role) const
{
    if (_item == nullptr
        || checkIndex(index) == false) {

        return QVariant();
    }

    const auto& field = _data.at(index.row());

    if (role == Qt::DisplayRole
        || (role == Qt::EditRole && field.isParent == false)) {

        switch (static_cast<ColumnId>(index.column())) {
        case NAME_COLUMN:
            return field.name;

        case TYPE_COLUMN:
            if (role == Qt::DisplayRole) {
                return TYPE_STRINGS.at(int(field.type));
            }
            else {
                return int(field.type);
            }

        case DEFAULT_VALUE_COLUMN:
            return field.defaultValue;

        case COMMENT_COLUMN:
            return field.comment;
        }
    }

    return QVariant();
}

bool StructFieldsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (_item == nullptr
        || checkIndex(index) == false
        || role != Qt::EditRole) {

        return false;
    }

    const auto& field = _data.at(index.row());
    if (field.isParent) {
        return false;
    }

    size_t fieldIndex = index.row() - _data.fieldsInParent();

    switch (static_cast<ColumnId>(index.column())) {
    case NAME_COLUMN:
        return _item->structFieldList()->editSelectedList_setName(
            fieldIndex, value.toString().toStdString());

    case TYPE_COLUMN:
        return _item->structFieldList()->editSelectedList_setType(
            fieldIndex, static_cast<EN::DataType>(value.toInt()));

    case DEFAULT_VALUE_COLUMN:
        return _item->structFieldList()->editSelectedList_setDefaultValue(
            fieldIndex, value.toString().toStdString());

    case COMMENT_COLUMN:
        return _item->structFieldList()->editSelectedList_setComment(
            fieldIndex, value.toString().toStdString());
    }

    return false;
}
