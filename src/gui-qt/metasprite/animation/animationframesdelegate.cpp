/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationframesdelegate.h"
#include "animationframesmodel.h"

#include "gui-qt/common/abstractidmaplistmodel.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include <QComboBox>
#include <QCompleter>
#include <QLineEdit>
#include <QSpinBox>

using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationFramesDelegate::AnimationFramesDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
    , _document(nullptr)
{
}

void AnimationFramesDelegate::setDocument(AbstractMsDocument* document)
{
    _document = document;
}

QWidget* AnimationFramesDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&,
                                               const QModelIndex& index) const
{
    using Column = AnimationFramesModel::Column;

    if (!index.isValid()
        || index.column() < 0 || index.column() > AnimationFramesModel::N_COLUMNS) {
        return nullptr;
    }

    switch ((Column)index.column()) {
    case Column::FRAME: {
        QLineEdit* editor = new QLineEdit(parent);
        editor->setValidator(new IdstringValidator(editor));
        editor->setAutoFillBackground(true);

        if (_document != nullptr) {
            QCompleter* completer = new QCompleter(editor);
            completer->setCompletionRole(Qt::DisplayRole);
            completer->setCaseSensitivity(Qt::CaseInsensitive);
            completer->setModel(_document->frameListModel());

            editor->setCompleter(completer);
        }
        return editor;
    }

    case Column::FLIP: {
        QComboBox* editor = new QComboBox(parent);
        editor->addItems(AnimationFramesModel::FLIP_STRINGS);
        return editor;
    }

    case Column::DURATION: {
        QSpinBox* editor = new QSpinBox(parent);
        editor->setFrame(false);
        editor->setAutoFillBackground(true);
        editor->setMinimum(1);
        editor->setMaximum(255);
        return editor;
    }
    }

    return nullptr;
}

void AnimationFramesDelegate::setEditorData(QWidget* editor,
                                            const QModelIndex& index) const
{
    using Column = AnimationFramesModel::Column;

    if (!index.isValid()
        || index.column() < 0 || index.column() > AnimationFramesModel::N_COLUMNS) {
        return;
    }

    QVariant value = index.data(Qt::EditRole);

    switch ((Column)index.column()) {
    case Column::FRAME: {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
        lineEdit->setText(value.toString());
        break;
    }

    case Column::FLIP: {
        QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(value.toInt());
        break;
    }

    case Column::DURATION: {
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor);
        spinBox->setValue(value.toInt());
        break;
    }
    }
}

void AnimationFramesDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                           const QModelIndex& index) const
{
    using Column = AnimationFramesModel::Column;

    if (!index.isValid()
        || index.column() < 0 || index.column() > AnimationFramesModel::N_COLUMNS) {
        return;
    }

    switch ((Column)index.column()) {
    case Column::FRAME: {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
        model->setData(index, lineEdit->text(), Qt::EditRole);
        break;
    }

    case Column::FLIP: {
        QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
        model->setData(index, comboBox->currentIndex(), Qt::EditRole);
        break;
    }

    case Column::DURATION: {
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        model->setData(index, spinBox->value(), Qt::EditRole);
        break;
    }
    }
}

void AnimationFramesDelegate::updateEditorGeometry(QWidget* editor,
                                                   const QStyleOptionViewItem& option,
                                                   const QModelIndex&) const
{
    editor->setGeometry(option.rect);
}
