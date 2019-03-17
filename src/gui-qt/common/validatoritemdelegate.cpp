/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "validatoritemdelegate.h"
#include <QLineEdit>

using namespace UnTech::GuiQt;

ValidatorItemDelegate::ValidatorItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* ValidatorItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QVariant valData = index.data(ValidatorRole);
    if (auto* v = valData.value<QValidator*>()) {
        auto* lineEdit = new QLineEdit(parent);
        lineEdit->setValidator(v);
        lineEdit->setFocusPolicy(Qt::WheelFocus);
        lineEdit->setAutoFillBackground(true);
        lineEdit->setFrame(false);

        return lineEdit;
    }
    else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void ValidatorItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (auto* le = qobject_cast<QLineEdit*>(editor)) {
        QVariant data = index.data(Qt::EditRole);
        le->setText(data.toString());
    }
    else {
        return QStyledItemDelegate::setEditorData(editor, index);
    }
}

void ValidatorItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (auto* le = qobject_cast<QLineEdit*>(editor)) {
        model->setData(index, le->text(), Qt::EditRole);
    }
    else {
        return QStyledItemDelegate::setModelData(editor, model, index);
    }
}
