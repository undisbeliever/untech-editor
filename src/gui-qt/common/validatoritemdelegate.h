/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QStyledItemDelegate>

namespace UnTech {
namespace GuiQt {

class ValidatorItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    constexpr static int ValidatorRole = Qt::UserRole + 0x42;

public:
    ValidatorItemDelegate(QObject* parent = nullptr);
    ~ValidatorItemDelegate() = default;

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const final;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const final;
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model,
                              const QModelIndex& index) const final;
};

}
}
