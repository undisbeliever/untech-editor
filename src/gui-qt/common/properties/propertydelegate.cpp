/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertydelegate.h"
#include "abstractpropertymodel.h"
#include "property.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

#include "listitemwidget.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/common/widgets/colorinputwidget.h"
#include "gui-qt/common/widgets/filenameinputwidget.h"
#include "gui-qt/common/widgets/pointwidget.h"
#include "gui-qt/common/widgets/rectwidget.h"
#include "gui-qt/common/widgets/sizewidget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QLineEdit>
#include <QSpinBox>

using namespace UnTech::GuiQt;
using Type = PropertyType;

PropertyDelegate::PropertyDelegate(QObject* parent)
    : QItemDelegate(parent)
{
}

QRect PropertyDelegate::checkBoxRect(const QStyleOptionViewItem& option) const
{
    QStyleOption opt = option;
    QStyle* style = option.widget ? option.widget->style() : QApplication::style();
    return style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, option.widget);
}

QSize PropertyDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto sizeForText = [option](QString text) {
        if (text.isEmpty()) {
            text = QStringLiteral(" ");
        }

        QStyle* style = option.widget ? option.widget->style() : QApplication::style();

        int frameHMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
        int textHMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
        int extendWidth = frameHMargin * 3 + textHMargin * 2;

        int extendHeight = style->pixelMetric(QStyle::PM_FocusFrameVMargin) * 2;

        QRect b = option.fontMetrics.boundingRect(text);
        return QSize(b.width() + extendWidth, b.height() + extendHeight);
    };

    const auto* model = qobject_cast<const AbstractPropertyModel*>(index.model());

    if (model && model->propertyForIndex(index).type == Type::BOOLEAN) {
        QRect rect = checkBoxRect(option);
        return QSize(rect.width() + 2, rect.height() + 2);
    }
    else {
        QVariant value = index.data(Qt::DisplayRole);
        return sizeForText(value.toString());
    }

    return QItemDelegate::sizeHint(option, index);
}

void PropertyDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    const auto* model = qobject_cast<const AbstractPropertyModel*>(index.model());
    const auto& property = model && index.isValid() ? model->propertyForIndex(index)
                                                    : AbstractPropertyModel::blankProperty;

    drawBackground(painter, option, index);

    QVariant value = index.data(Qt::DisplayRole);

    if (property.id < 0) {
        drawDisplay(painter, option, option.rect, value.toString());
    }
    else if (property.isList && model && !model->isListItem(index)) {
        // container node of a list type
        QStyleOptionViewItem opt = option;
        opt.font.setItalic(true);
        drawDisplay(painter, opt, option.rect, value.toString());
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
        case Type::COLOR:
        case Type::POINT:
        case Type::SIZE:
        case Type::RECT:
        case Type::COMBO: {
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

bool PropertyDelegate::editorEvent(QEvent* event, QAbstractItemModel* aModel,
                                   const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_ASSERT(aModel);
    const auto* pModel = qobject_cast<const AbstractPropertyModel*>(index.model());

    if (pModel && pModel->propertyForIndex(index).type == Type::BOOLEAN) {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* e = static_cast<QMouseEvent*>(event);

            QRect checkRect = checkBoxRect(option);
            if (checkRect.contains(e->pos())) {
                QVariant value = index.data(Qt::EditRole);
                aModel->setData(index, !value.toBool(), Qt::EditRole);
            }
            return false;
        }
    }

    return QItemDelegate::editorEvent(event, aModel, option, index);
}

QWidget* PropertyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
    const auto* model = qobject_cast<const AbstractPropertyModel*>(index.model());
    if (model == nullptr) {
        return nullptr;
    }
    const auto& property = model->propertyForIndex(index);
    if (property.id < 0) {
        return nullptr;
    }

    QWidget* editor = createEditorWidget(parent, model, index, property);
    if (editor) {
        connect(model, &AbstractPropertyModel::requestCloseEditors,
                editor, [=]() {
                    // Close editor when underlying data is about to change

                    // I know this is bad but QSignalMapper is deprecated
                    emit const_cast<PropertyDelegate*>(this)->closeEditor(editor, EndEditHint::NoHint);
                });
    }
    return editor;
}

QWidget* PropertyDelegate::createEditorWidget(QWidget* parent, const AbstractPropertyModel* model,
                                              const QModelIndex& index, const Property& property) const
{
    if (property.isList && model && !model->isListItem(index)) {
        // container node of a list type
        ListItemWidget* li = new ListItemWidget(model, index, parent);
        li->setAutoFillBackground(true);

        connect(li, &ListItemWidget::listEdited,
                this, &PropertyDelegate::commitEditor);
        return li;
    }

    // leaf node
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

    case Type::POINT: {
        PointWidget* pw = new PointWidget(parent);
        pw->setAutoFillBackground(true);
        return pw;
    }

    case Type::SIZE: {
        SizeWidget* sw = new SizeWidget(parent);
        sw->setAutoFillBackground(true);
        return sw;
    }

    case Type::RECT: {
        RectWidget* rw = new RectWidget(parent);
        rw->setAutoFillBackground(true);
        return rw;
    }

    case Type::COMBO: {
        QComboBox* cb = new QComboBox(parent);
        cb->setAutoFillBackground(true);
        cb->setFrame(false);
        return cb;
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
    const auto* model = qobject_cast<const AbstractPropertyModel*>(index.model());
    if (model == nullptr) {
        return;
    }
    const auto& property = model->propertyForIndex(index);
    if (property.id < 0) {
        return;
    }

    const QVariant data = index.data(Qt::EditRole);

    if (property.isList && model && !model->isListItem(index)) {
        // container node of a list type
        ListItemWidget* li = qobject_cast<ListItemWidget*>(editor);
        li->setStringList(data.toStringList());
        return;
    }

    auto params = model->propertyParametersForIndex(index);

    switch (property.type) {
    case Type::BOOLEAN: {
        QCheckBox* cb = qobject_cast<QCheckBox*>(editor);
        cb->setChecked(data.toBool());
    } break;

    case Type::INTEGER:
    case Type::UNSIGNED: {
        QSpinBox* sb = qobject_cast<QSpinBox*>(editor);
        if (params.first.isValid()) {
            sb->setMinimum(params.first.toInt());
        }
        if (params.second.isValid()) {
            sb->setMaximum(params.second.toInt());
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
        if (params.first.type() == QVariant::StringList) {
            QCompleter* c = new QCompleter(params.first.toStringList(), le);
            c->setCaseSensitivity(Qt::CaseInsensitive);
            le->setCompleter(c);
        }
        le->setText(data.toString());
    } break;

    case Type::FILENAME:
    case Type::FILENAME_LIST: {
        FilenameInputWidget* fi = qobject_cast<FilenameInputWidget*>(editor);
        if (params.first.isValid()) {
            fi->setDialogFilter(params.first.toString());
        }
        fi->setFilename(data.toString());
    } break;

    case Type::COLOR: {
        ColorInputWidget* ci = qobject_cast<ColorInputWidget*>(editor);
        ci->setColor(data.value<QColor>());
    } break;

    case Type::POINT: {
        PointWidget* pw = qobject_cast<PointWidget*>(editor);
        if (params.first.type() == QVariant::Point) {
            pw->setMinimum(params.first.toPoint());
        }
        if (params.second.type() == QVariant::Point) {
            pw->setMaximum(params.second.toPoint());
        }
        pw->setValue(data.toPoint());
    } break;

    case Type::SIZE: {
        SizeWidget* sw = qobject_cast<SizeWidget*>(editor);
        if (params.first.type() == QVariant::Size) {
            sw->setMinimum(params.first.toSize());
        }
        if (params.second.type() == QVariant::Size) {
            sw->setMaximum(params.second.toSize());
        }
        sw->setValue(data.toSize());
    } break;

    case Type::RECT: {
        RectWidget* rw = qobject_cast<RectWidget*>(editor);
        if (params.first.type() == QVariant::Rect) {
            rw->setRange(params.first.toRect());
        }
        if (params.second.type() == QVariant::Size) {
            rw->setMaxRectSize(params.second.toSize());
        }
        rw->setValue(data.toRect());
    } break;

    case Type::COMBO: {
        const QStringList displayList = params.first.toStringList();
        const QVariantList dataList = params.second.toList();

        QComboBox* cb = qobject_cast<QComboBox*>(editor);
        cb->clear();

        if (displayList.size() == dataList.size()) {
            for (int i = 0; i < displayList.size(); i++) {
                cb->addItem(displayList.at(i), dataList.at(i));
            }
        }
        else {
            for (int i = 0; i < displayList.size(); i++) {
                cb->addItem(displayList.at(i), displayList.at(i));
            }
        }
        int index = cb->findData(data);
        cb->setCurrentIndex(index);
    } break;
    }
}

void PropertyDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    const auto* pModel = qobject_cast<const AbstractPropertyModel*>(index.model());
    if (pModel == nullptr) {
        return;
    }
    const auto& property = pModel->propertyForIndex(index);
    if (property.id < 0) {
        return;
    }

    if (property.isList && pModel && !pModel->isListItem(index)) {
        // container node of a list type
        ListItemWidget* li = qobject_cast<ListItemWidget*>(editor);
        model->setData(index, li->stringList(), Qt::EditRole);

        return;
    }

    // leaf node
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

    case Type::POINT: {
        PointWidget* pw = qobject_cast<PointWidget*>(editor);
        model->setData(index, pw->value(), Qt::EditRole);
    } break;

    case Type::SIZE: {
        SizeWidget* sw = qobject_cast<SizeWidget*>(editor);
        model->setData(index, sw->value(), Qt::EditRole);
    } break;

    case Type::RECT: {
        RectWidget* rw = qobject_cast<RectWidget*>(editor);
        model->setData(index, rw->value(), Qt::EditRole);
    } break;

    case Type::COMBO: {
        QComboBox* cb = qobject_cast<QComboBox*>(editor);
        model->setData(index, cb->currentData(), Qt::EditRole);
    } break;
    }
}

void PropertyDelegate::updateEditorGeometry(QWidget* editor,
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
