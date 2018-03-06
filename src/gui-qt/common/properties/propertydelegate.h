/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "propertymanager.h"
#include <QStyledItemDelegate>

namespace UnTech {
namespace GuiQt {
class PropertyManager;

class PropertyDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    PropertyDelegate(QObject* parent = nullptr);
    ~PropertyDelegate() = default;

    virtual QString displayText(const QVariant& value, const QLocale& locale) const final;

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const final;

    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const final;

    virtual void setModelData(QWidget* editor, QAbstractItemModel* model,
                              const QModelIndex& index) const final;

private:
    const PropertyManager::Property& propertyForIndex(const QModelIndex& index) const;

private slots:
    void commitEditor();
};
}
}
