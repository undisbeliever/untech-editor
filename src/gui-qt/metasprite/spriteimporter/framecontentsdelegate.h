/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QStyledItemDelegate>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;

class FrameContentsDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    enum DataType {
        UPOINT = 1,
        URECT,
        OBJECT_SIZE,
        ACTION_POINT_PARAMETER,
        ENTITY_HITBOX_TYPE
    };
    const static int DataTypeRole = Qt::UserRole + 1;
    const static int RangeRole = Qt::UserRole + 2;

public:
    explicit FrameContentsDelegate(QObject* parent = nullptr);
    ~FrameContentsDelegate() = default;

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const final;

    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const final;

    virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const final;
};
}
}
}
}
