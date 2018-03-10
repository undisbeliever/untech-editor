/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "listitemwidget.h"
#include "propertymanager.h"
#include "propertymodel.h"

#include <QFileDialog>
#include <QHBoxLayout>

using namespace UnTech::GuiQt;
using Type = PropertyType;

ListItemWidget::ListItemWidget(const PropertyManager* manager,
                               int propertyIndex, QWidget* parent)
    : QWidget(parent)
    , _manager(manager)
    , _propertyIndex(propertyIndex)
    , _stringList()
{
    Q_ASSERT(manager);
    Q_ASSERT(propertyIndex >= 0);
    Q_ASSERT(propertyIndex < manager->propertiesList().size());

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
    const auto& property = _manager->propertiesList().at(_propertyIndex);

    switch (property.type) {
    case Type::BOOLEAN:
    case Type::INTEGER:
    case Type::UNSIGNED:
    case Type::STRING:
    case Type::IDSTRING:
    case Type::FILENAME:
    case Type::COLOR:
    case Type::COMBO:
        break;

    case Type::STRING_LIST:
    case Type::IDSTRING_LIST: {
        _stringList.append(QString());
        emit listEdited();
    } break;

    case Type::FILENAME_LIST: {
        QVariant filter = property.parameter1;
        QVariant param2 = property.parameter2;

        _manager->updateParameters(property.id, filter, param2);

        const QStringList filenames = QFileDialog::getOpenFileNames(
            this, QString(), QString(), filter.toString());

        _stringList.append(filenames);
        emit listEdited();
    } break;
    }
}
