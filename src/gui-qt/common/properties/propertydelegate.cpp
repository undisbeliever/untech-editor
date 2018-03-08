/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertydelegate.h"
#include "propertymanager.h"
#include "propertymodel.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/common/widgets/colorinputwidget.h"
#include "gui-qt/common/widgets/filenameinputwidget.h"
#include <QCheckBox>
#include <QCompleter>
#include <QLineEdit>
#include <QSpinBox>

using namespace UnTech::GuiQt;
using Type = PropertyManager::Type;

PropertyDelegate::PropertyDelegate(QObject* parent)
    : QItemDelegate(parent)
{
}

const PropertyManager::Property& PropertyDelegate::propertyForIndex(const QModelIndex& index) const
{
    const PropertyModel* model = qobject_cast<const PropertyModel*>(index.model());
    if (index.isValid() == false || model == nullptr) {
        return PropertyManager::blankProperty;
    }

    const auto& pl = model->manager()->propertiesList();

    if (index.internalId() == PropertyModel::ROOT_INTERNAL_ID) {
        if (index.row() < pl.size()) {
            return pl.at(index.row());
        }
    }
    else {
        if (index.internalId() < (unsigned)pl.size()
            && pl.at(index.internalId()).isList) {

            return pl.at(index.internalId());
        }
    }

    return PropertyManager::blankProperty;
}

QRect PropertyDelegate::checkBoxRect(const QStyleOptionViewItem& option) const
{
    QStyleOption opt = option;
    QStyle* style = option.widget ? option.widget->style() : QApplication::style();
    return style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, option.widget);
}

QString PropertyDelegate::listCountString(const QVariant& value) const
{
    auto itemCountString = [](int s) {
        if (s != 1) {
            return tr("(%1 items)").arg(s);
        }
        else {
            return tr("(1 item)");
        }
    };

    if (value.type() == QVariant::List) {
        return itemCountString(value.toList().size());
    }
    else if (value.type() == QVariant::StringList) {
        return itemCountString(value.toStringList().size());
    }
    else {
        return value.toString();
    }
}

QSize PropertyDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto sizeForText = [option](const QString& text) {
        QStyle* style = option.widget ? option.widget->style() : QApplication::style();

        int frameHMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
        int textHMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
        int extendWidth = frameHMargin * 3 + textHMargin * 2;

        int extendHeight = style->pixelMetric(QStyle::PM_FocusFrameVMargin) * 2;

        QRect b = option.fontMetrics.boundingRect(text);
        return QSize(b.width() + extendWidth, b.height() + extendHeight);
    };

    const auto& property = propertyForIndex(index);

    if (index.column() == PropertyModel::PROPERTY_COLUMN) {
        QVariant value = index.data(Qt::DisplayRole);
        return sizeForText(value.toString());
    }
    else {
        if (property.type == Type::BOOLEAN) {
            QRect rect = checkBoxRect(option);
            return QSize(rect.width() + 2, rect.height() + 2);
        }
        else if (property.isList && index.internalId() == PropertyModel::ROOT_INTERNAL_ID) {
            return sizeForText(QStringLiteral("(xxx items)"));
        }
        else {
            QVariant value = index.data(Qt::DisplayRole);
            return sizeForText(value.toString());
        }
    }

    return QItemDelegate::sizeHint(option, index);
}

void PropertyDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    const auto& property = propertyForIndex(index);

    drawBackground(painter, option, index);

    QVariant value = index.data(Qt::DisplayRole);

    if (index.column() == PropertyModel::PROPERTY_COLUMN) {
        drawDisplay(painter, option, option.rect, value.toString());
    }
    else if (property.isList && index.internalId() == PropertyModel::ROOT_INTERNAL_ID) {
        // middle node of a list type
        QStyleOptionViewItem opt = option;
        opt.font.setItalic(true);
        drawDisplay(painter, opt, option.rect, listCountString(value));
    }
    else {
        // leaf node
        switch (property.type) {
        case Type::BOOLEAN: {
            QRect rect = checkBoxRect(option);
            Qt::CheckState state = value.toBool() ? Qt::Checked : Qt::Unchecked;
            drawCheck(painter, option, rect, state);
        } break;

        case Type::INTEGER:
        case Type::UNSIGNED:
        case Type::STRING:
        case Type::STRING_LIST:
        case Type::IDSTRING:
        case Type::IDSTRING_LIST:
        case Type::COLOR: {
            drawDisplay(painter, option, option.rect, value.toString());
        } break;

        case Type::FILENAME:
        case Type::FILENAME_LIST: {
            QStyleOptionViewItem opt = option;
            opt.textElideMode = Qt::ElideLeft;
            drawDisplay(painter, opt, option.rect, value.toString());
        } break;
        }
    }

    drawFocus(painter, option, option.rect);
}

bool PropertyDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                   const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_ASSERT(model);

    if (index.column() == PropertyModel::VALUE_COLUMN) {
        const auto& property = propertyForIndex(index);

        if (property.type == Type::BOOLEAN) {
            if (event->type() == QEvent::MouseButtonRelease) {
                QMouseEvent* e = static_cast<QMouseEvent*>(event);

                QRect checkRect = checkBoxRect(option);
                if (checkRect.contains(e->pos())) {
                    QVariant value = index.data(Qt::EditRole);
                    model->setData(index, !value.toBool(), Qt::EditRole);
                }
                return false;
            }
        }
    }

    return QItemDelegate::editorEvent(event, model, option, index);
}

QWidget* PropertyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
    const auto& property = propertyForIndex(index);

    if (property.isList && index.internalId() == PropertyModel::ROOT_INTERNAL_ID) {
        return nullptr;
    }

    switch (property.type) {
    case Type::BOOLEAN: {
        QCheckBox* cb = new QCheckBox(parent);
        cb->setAutoFillBackground(true);
        connect(cb, &QCheckBox::clicked,
                this, &PropertyDelegate::commitEditor);
        return cb;
    }

    case Type::INTEGER: {
        QSpinBox* sb = new QSpinBox(parent);
        sb->setAutoFillBackground(true);
        sb->setFrame(false);
        sb->setMinimum(INT_MIN);
        sb->setMaximum(INT_MAX);
        return sb;
    }

    case Type::UNSIGNED: {
        QSpinBox* sb = new QSpinBox(parent);
        sb->setAutoFillBackground(true);
        sb->setFrame(false);
        sb->setMinimum(0);
        sb->setMaximum(INT_MAX);
        return sb;
    }

    case Type::STRING:
    case Type::STRING_LIST: {
        QLineEdit* le = new QLineEdit(parent);
        le->setAutoFillBackground(true);
        le->setFrame(false);
        return le;
    }

    case Type::IDSTRING:
    case Type::IDSTRING_LIST: {
        QLineEdit* le = new QLineEdit(parent);
        le->setAutoFillBackground(true);
        le->setFrame(false);
        le->setValidator(new IdstringValidator(le));
        return le;
    }

    case Type::FILENAME:
    case Type::FILENAME_LIST: {
        FilenameInputWidget* fi = new FilenameInputWidget(parent);
        fi->setAutoFillBackground(true);
        fi->setFrame(false);
        connect(fi, &FilenameInputWidget::fileSelected,
                this, &PropertyDelegate::commitEditor);
        return fi;
    }

    case Type::COLOR: {
        ColorInputWidget* ci = new ColorInputWidget(parent);
        ci->setAutoFillBackground(true);
        ci->setFrame(false);
        connect(ci, &ColorInputWidget::colorSelected,
                this, &PropertyDelegate::commitEditor);
        return ci;
    }
    }

    return nullptr;
}

void PropertyDelegate::commitEditor()
{
    QWidget* editor = qobject_cast<QWidget*>(sender());
    emit commitData(editor);
}

void PropertyDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    const PropertyModel* model = qobject_cast<const PropertyModel*>(index.model());
    if (model == nullptr) {
        return;
    }

    const auto& property = propertyForIndex(index);
    const QVariant data = index.data(Qt::EditRole);

    if (property.isList && index.internalId() == PropertyModel::ROOT_INTERNAL_ID) {
        return;
    }

    QVariant param1 = property.parameter1;
    QVariant param2 = property.parameter2;
    model->manager()->updateParameters(property.id, param1, param2);

    switch (property.type) {
    case Type::BOOLEAN: {
        QCheckBox* cb = qobject_cast<QCheckBox*>(editor);
        cb->setChecked(data.toBool());
    } break;

    case Type::INTEGER:
    case Type::UNSIGNED: {
        QSpinBox* sb = qobject_cast<QSpinBox*>(editor);
        if (param1.isValid()) {
            sb->setMinimum(param1.toInt());
        }
        if (param2.isValid()) {
            sb->setMaximum(param2.toInt());
        }
        sb->setValue(data.toInt());
    } break;

    case Type::STRING:
    case Type::STRING_LIST: {
        QLineEdit* le = qobject_cast<QLineEdit*>(editor);
        le->setText(data.toString());
    } break;

    case Type::IDSTRING:
    case Type::IDSTRING_LIST: {
        QLineEdit* le = qobject_cast<QLineEdit*>(editor);

        if (QCompleter* c = le->completer()) {
            le->setCompleter(nullptr);
            c->deleteLater();
        };
        if (param1.type() == QVariant::StringList) {
            QCompleter* c = new QCompleter(param1.toStringList(), le);
            c->setCaseSensitivity(Qt::CaseInsensitive);
            le->setCompleter(c);
        }
        le->setText(data.toString());
    } break;

    case Type::FILENAME:
    case Type::FILENAME_LIST: {
        FilenameInputWidget* fi = qobject_cast<FilenameInputWidget*>(editor);
        if (param1.isValid()) {
            fi->setDialogFilter(param1.toString());
        }
        fi->setFilename(data.toString());
    } break;

    case Type::COLOR: {
        ColorInputWidget* ci = qobject_cast<ColorInputWidget*>(editor);
        ci->setColor(data.value<QColor>());
    } break;
    }
}

void PropertyDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    const auto& property = propertyForIndex(index);

    if (property.isList && index.internalId() == PropertyModel::ROOT_INTERNAL_ID) {
        return;
    }

    switch (property.type) {
    case Type::BOOLEAN: {
        QCheckBox* cb = qobject_cast<QCheckBox*>(editor);
        model->setData(index, cb->isChecked(), Qt::EditRole);
    } break;

    case Type::INTEGER:
    case Type::UNSIGNED: {
        QSpinBox* sb = qobject_cast<QSpinBox*>(editor);
        model->setData(index, sb->value(), Qt::EditRole);
    } break;

    case Type::STRING:
    case Type::IDSTRING:
    case Type::STRING_LIST:
    case Type::IDSTRING_LIST: {
        QLineEdit* le = qobject_cast<QLineEdit*>(editor);
        model->setData(index, le->text(), Qt::EditRole);
    } break;

    case Type::FILENAME:
    case Type::FILENAME_LIST: {
        FilenameInputWidget* fi = qobject_cast<FilenameInputWidget*>(editor);
        model->setData(index, fi->filename(), Qt::EditRole);
    } break;

    case Type::COLOR: {
        ColorInputWidget* ci = qobject_cast<ColorInputWidget*>(editor);
        model->setData(index, ci->color(), Qt::EditRole);
    } break;
    }
}
