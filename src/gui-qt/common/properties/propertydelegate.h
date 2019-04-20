/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QItemDelegate>

namespace UnTech {
namespace GuiQt {
struct Property;
class AbstractPropertyModel;

/*
 * NOTICE
 * ======
 *
 * All editors created PropertyDelegate MUST be closed before this object can
 * be deleted safely.
 */

class PropertyDelegate : public QItemDelegate {
    Q_OBJECT

public:
    PropertyDelegate(QObject* parent = nullptr);
    ~PropertyDelegate() = default;

    static QString colorText(const QColor& color);

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
    QWidget* createEditorWidget(QWidget* parent, const AbstractPropertyModel* model,
                                const QModelIndex& index, const Property& property) const;

    QRect checkBoxRect(const QStyleOptionViewItem& option) const;

private slots:
    void commitEditor();
};
}
}
