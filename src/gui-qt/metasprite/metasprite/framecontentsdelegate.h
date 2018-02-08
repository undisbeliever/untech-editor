/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QStyledItemDelegate>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;

class FrameContentsDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    enum DataType {
        MS8POINT = 1,
        MS8RECT,
        OBJECT_TILE,
        ACTION_POINT_PARAMETER,
        ENTITY_HITBOX_TYPE
    };
    const static int DataTypeRole = Qt::UserRole + 1;
    const static int DocumentRole = Qt::UserRole + 2;

public:
    explicit FrameContentsDelegate(QObject* parent = nullptr);
    ~FrameContentsDelegate() = default;

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const final;

    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const final;

    virtual void setModelData(QWidget* editor, QAbstractItemModel* model,
                              const QModelIndex& index) const final;

    virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const final;
};
}
}
}
}
