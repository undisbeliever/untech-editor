/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QItemDelegate>

namespace UnTech {
namespace GuiQt {
struct Property;

class PropertyDelegate : public QItemDelegate {
    Q_OBJECT

public:
    PropertyDelegate(QObject* parent = nullptr);
    ~PropertyDelegate() = default;

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final;

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const final;

    virtual bool editorEvent(QEvent* event, QAbstractItemModel* model,
                             const QStyleOptionViewItem& option, const QModelIndex& index) final;

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const final;

    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const final;

    virtual void setModelData(QWidget* editor, QAbstractItemModel* model,
                              const QModelIndex& index) const final;

    virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const final;

private:
    QRect checkBoxRect(const QStyleOptionViewItem& option) const;

private slots:
    void commitEditor();
};
}
}
