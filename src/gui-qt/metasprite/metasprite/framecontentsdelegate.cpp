/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecontentsdelegate.h"
#include "document.h"
#include "framecontentsmodel.h"
#include "frameobjecttilewidget.h"
#include "selection.h"

#include "gui-qt/common/widgets/enumcombobox.h"
#include "gui-qt/common/widgets/ms8pointwidget.h"
#include "gui-qt/common/widgets/ms8rectwidget.h"
#include <QSpinBox>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;
using ActionPointParameter = UnTech::MetaSprite::ActionPointParameter;
using EntityHitboxType = UnTech::MetaSprite::EntityHitboxType;

FrameContentsDelegate::FrameContentsDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* FrameContentsDelegate::createEditor(QWidget* parent,
                                             const QStyleOptionViewItem&,
                                             const QModelIndex& index) const
{
    if (!index.isValid()) {
        return nullptr;
    }

    int type = index.data(DataTypeRole).toInt();

    switch ((DataType)type) {
    case MS8POINT: {
        Ms8pointWidget* editor = new Ms8pointWidget(parent);
        editor->setAutoFillBackground(true);
        return editor;
    }

    case MS8RECT: {
        Ms8rectWidget* editor = new Ms8rectWidget(parent);
        editor->setAutoFillBackground(true);
        return editor;
    }

    case OBJECT_TILE: {
        FrameObjectTileWidget* editor = new FrameObjectTileWidget(parent);
        editor->setAutoFillBackground(true);
        return editor;
    }

    case ACTION_POINT_PARAMETER: {
        QSpinBox* editor = new QSpinBox(parent);
        editor->setFrame(false);
        editor->setAutoFillBackground(true);
        editor->setMinimum(ActionPointParameter::MIN);
        editor->setMaximum(ActionPointParameter::MAX);
        return editor;
    }

    case ENTITY_HITBOX_TYPE: {
        EnumComboBox* editor = new EnumComboBox(parent);
        editor->populateData(EntityHitboxType::enumMap);
        return editor;
    }

    default:
        return nullptr;
    }
}

void FrameContentsDelegate::setEditorData(QWidget* editor,
                                          const QModelIndex& index) const
{
    if (!index.isValid()) {
        return;
    }

    int type = index.data(DataTypeRole).toInt();
    QVariant value = index.data(Qt::EditRole);

    switch ((DataType)type) {
    case MS8POINT: {
        Ms8pointWidget* pointEditor = qobject_cast<Ms8pointWidget*>(editor);
        pointEditor->setValue(value.toPoint());
    } break;

    case MS8RECT: {
        Ms8rectWidget* rectEditor = qobject_cast<Ms8rectWidget*>(editor);
        rectEditor->setValue(value.toRect());
    } break;

    case OBJECT_TILE: {
        FrameObjectTileWidget* objEditor = qobject_cast<FrameObjectTileWidget*>(editor);
        auto* document = index.data(DocumentRole).value<Document*>();
        if (document) {
            objEditor->setTilesetSize(*document->frameSet());
        }
        objEditor->setValue(value.toUInt());
    } break;

    case ENTITY_HITBOX_TYPE: {
        EnumComboBox* comboBox = qobject_cast<EnumComboBox*>(editor);
        comboBox->setCurrentEnum(value.toInt());
    } break;

    case ACTION_POINT_PARAMETER: {
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor);
        spinBox->setValue(value.toInt());
    } break;
    }
}

void FrameContentsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                         const QModelIndex& index) const
{
    int type = index.data(DataTypeRole).toInt();
    QVariant value = index.data(Qt::EditRole);

    switch ((DataType)type) {
    case MS8POINT: {
        Ms8pointWidget* pointEditor = qobject_cast<Ms8pointWidget*>(editor);
        model->setData(index, pointEditor->value(), Qt::EditRole);
    } break;

    case MS8RECT: {
        Ms8rectWidget* rectEditor = qobject_cast<Ms8rectWidget*>(editor);
        model->setData(index, rectEditor->value(), Qt::EditRole);
    } break;

    case OBJECT_TILE: {
        FrameObjectTileWidget* objEditor = qobject_cast<FrameObjectTileWidget*>(editor);
        model->setData(index, objEditor->value(), Qt::EditRole);
    } break;

    case ENTITY_HITBOX_TYPE: {
        EnumComboBox* comboBox = qobject_cast<EnumComboBox*>(editor);
        model->setData(index, comboBox->currentEnumInt(), Qt::EditRole);
    } break;

    case ACTION_POINT_PARAMETER: {
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        model->setData(index, spinBox->value(), Qt::EditRole);
    } break;
    }
}

void FrameContentsDelegate::updateEditorGeometry(QWidget* editor,
                                                 const QStyleOptionViewItem& option,
                                                 const QModelIndex&) const
{
    QRect r = option.rect;
    if (editor->minimumHeight() > 0) {
        r.setHeight(editor->minimumHeight());
    }

    QWidget* parent = editor->parentWidget();
    if (parent) {
        if (r.bottom() >= parent->height()) {
            r.moveBottom(parent->height() - 1);
        }
    }

    editor->setGeometry(r);
}
