/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecontentsdelegate.h"
#include "document.h"
#include "framecontentsmodel.h"
#include "selection.h"

#include "gui-qt/common/widgets/enumcombobox.h"
#include "gui-qt/common/widgets/pointwidget.h"
#include "gui-qt/common/widgets/rectwidget.h"
#include <QSpinBox>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;
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
    case UPOINT: {
        PointWidget* editor = new PointWidget(parent);
        editor->setAutoFillBackground(true);
        return editor;
    }

    case URECT: {
        RectWidget* editor = new RectWidget(parent);
        editor->setAutoFillBackground(true);
        return editor;
    }

    case OBJECT_SIZE: {
        QComboBox* editor = new QComboBox(parent);
        editor->addItem(tr("Small"));
        editor->addItem(tr("Large"));
        return editor;
    }

    case ACTION_POINT_PARAMETER: {
        QSpinBox* editor = new QSpinBox(parent);
        editor->setFrame(false);
        editor->setAutoFillBackground(true);
        editor->setMaximum(ActionPointParameter::MIN);
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
    case UPOINT: {
        QSize range = index.data(RangeRole).toSize();
        PointWidget* pointEditor = qobject_cast<PointWidget*>(editor);
        pointEditor->setMaximum(range);
        pointEditor->setValue(value.toPoint());
    } break;

    case URECT: {
        QSize range = index.data(RangeRole).toSize();
        RectWidget* rectEditor = qobject_cast<RectWidget*>(editor);
        rectEditor->setRange(range);
        rectEditor->setValue(value.toRect());
    } break;

    case OBJECT_SIZE: {
        QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(value.toInt());
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
