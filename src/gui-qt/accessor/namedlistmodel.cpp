/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "namedlistmodel.h"
#include "abstractaccessors.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/common/validatoritemdelegate.h"
#include "models/common/namedlist.h"

using namespace UnTech::GuiQt::Accessor;

UnTech::GuiQt::IdstringValidator* const NamedListModel::ID_STRING_VALIDATOR = new IdstringValidator;

NamedListModel::NamedListModel(QObject* parent)
    : QAbstractListModel(parent)
    , _accessor(nullptr)
    , _displayList()
{
}

void NamedListModel::setAccessor(AbstractNamedListAccessor* accessor)
{
    if (_accessor == accessor) {
        return;
    }

    if (_accessor) {
        _accessor->disconnect(this);
    }
    _accessor = accessor;

    resetDisplayList();

    if (_accessor) {
        connect(accessor, &AbstractNamedListAccessor::nameChanged,
                this, &NamedListModel::onNameChanged);
        connect(accessor, &AbstractNamedListAccessor::itemAdded,
                this, &NamedListModel::onItemAdded);
        connect(accessor, &AbstractNamedListAccessor::itemAboutToBeRemoved,
                this, &NamedListModel::onItemAboutToBeRemoved);
        connect(accessor, &AbstractNamedListAccessor::itemMoved,
                this, &NamedListModel::onItemMoved);
    }
}

void NamedListModel::resetDisplayList()
{
    beginResetModel();

    if (_accessor) {
        _displayList = _accessor->itemNames();
    }
    else {
        _displayList.clear();
    }

    endResetModel();
}

void NamedListModel::onNameChanged(size_t index)
{
    if (_accessor) {
        Q_ASSERT(index < unsigned(_displayList.size()));

        _displayList.replace(index, _accessor->itemName(index));

        QModelIndex mIndex = createIndex(index, 0);
        emit dataChanged(mIndex, mIndex);
    }
}

void NamedListModel::onItemAdded(size_t index)
{
    if (_accessor) {
        Q_ASSERT(index <= unsigned(_displayList.size()));

        _displayList.insert(index, _accessor->itemName(index));
        emit layoutChanged();
    }
}

void NamedListModel::onItemAboutToBeRemoved(size_t index)
{
    if (_accessor) {
        Q_ASSERT(index < unsigned(_displayList.size()));

        _displayList.removeAt(index);
        emit layoutChanged();
    }
}

void NamedListModel::onItemMoved(size_t from, size_t to)
{
    if (_accessor) {
        Q_ASSERT(from < unsigned(_displayList.size()));
        Q_ASSERT(to < unsigned(_displayList.size()));

        _displayList.move(from, to);
        emit layoutChanged();
    }
}

QModelIndex NamedListModel::toModelIndex(int i) const
{
    if (i < 0 || i >= _displayList.size()) {
        return QModelIndex();
    }
    return createIndex(i, 0);
}

size_t NamedListModel::toIndex(const QModelIndex& index) const
{
    const int row = index.row();
    if (row < 0 || row >= _displayList.size()) {
        return INT_MAX;
    }
    return row;
}

int NamedListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return _displayList.size();
}

Qt::ItemFlags NamedListModel::flags(const QModelIndex& index) const
{
    if (index.row() < 0 || index.row() >= _displayList.size()
        || _accessor == nullptr) {

        return Qt::ItemFlags();
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemNeverHasChildren;
}

QVariant NamedListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= _displayList.size()
        || _accessor == nullptr) {

        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return _displayList.at(index.row());
    }
    else if (role == Qt::EditRole) {
        return _accessor->itemName(index.row());
    }
    else if (role == ValidatorItemDelegate::ValidatorRole) {
        return QVariant::fromValue(ID_STRING_VALIDATOR);
    }

    return QVariant();
}

bool NamedListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.row() < 0 || index.row() >= _displayList.size()
        || _accessor == nullptr
        || role != Qt::EditRole) {

        return false;
    }

    const idstring name = value.toString().toStdString();
    if (name.isValid() == false) {
        return false;
    }

    return _accessor->edit_setName(index.row(), name);
}
