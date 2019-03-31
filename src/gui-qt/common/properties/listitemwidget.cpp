/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "listitemwidget.h"
#include "abstractpropertymodel.h"

#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>

using namespace UnTech::GuiQt;
using Type = PropertyType;

ListItemWidget::ListItemWidget(const AbstractPropertyModel* model,
                               const QModelIndex& index, QWidget* parent)
    : QWidget(parent)
    , _model(model)
    , _index(index)
    , _stringList()
{
    Q_ASSERT(model);
    Q_ASSERT(model->propertyForIndex(index).isList);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(1);
    layout->setMargin(0);

    _label = new QLabel(this);
    QFont f = _label->font();
    f.setItalic(true);
    _label->setFont(f);
    layout->addWidget(_label);

    _addButton = new QToolButton(this);
    _addButton->setToolTip(tr("Add Item"));
    _addButton->setIcon(QIcon(":/icons/add.svg"));
    layout->addWidget(_addButton);

    connect(_addButton, &QToolButton::clicked,
            this, &ListItemWidget::onAddButtonClicked);
}

void ListItemWidget::setStringList(const QStringList& list)
{
    _stringList = list;
    updateGui();
}

void ListItemWidget::updateGui()
{
    int s = _stringList.size();
    if (s != 1) {
        _label->setText(tr("<i>(%1 items)</i>").arg(s));
    }
    else {
        _label->setText(tr("<i>(1 item)</i>"));
    }
}

void ListItemWidget::onAddButtonClicked()
{
    const auto& property = _model->propertyForIndex(_index);

    switch (property.type) {
    case Type::BOOLEAN:
    case Type::INTEGER:
    case Type::UNSIGNED:
    case Type::STRING:
    case Type::IDSTRING:
    case Type::FILENAME:
    case Type::COLOR:
    case Type::POINT:
    case Type::SIZE:
    case Type::RECT:
    case Type::COMBO:
    case Type::COLOR_COMBO:
        break;

    case Type::STRING_LIST:
    case Type::IDSTRING_LIST: {
        _stringList.append(QString());
        emit listEdited();
    } break;

    case Type::FILENAME_LIST: {
        auto params = _model->propertyParametersForIndex(_index);

        // DontUseNativeDialog is required to prevent a segfault on my Win7 VM
        QStringList filenames = QFileDialog::getOpenFileNames(
            this, QString(), QString(), params.first.toString(),
            nullptr, QFileDialog::DontUseNativeDialog);

        for (const QString& fn : filenames) {
            _stringList.append(QDir::toNativeSeparators(fn));
        }

        emit listEdited();
    } break;
    }
}
