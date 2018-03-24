/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "genericpropertieswidget.h"
#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/resources/genericpropertieswidget.ui.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources;

GenericPropertiesWidget::GenericPropertiesWidget(AbstractPropertyManager* manager, QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::GenericPropertiesWidget>())
    , _manager(manager)
{
    Q_ASSERT(manager);

    _ui->setupUi(this);
    _ui->propertyView->setPropertyManager(manager);
}

GenericPropertiesWidget::~GenericPropertiesWidget() = default;

void GenericPropertiesWidget::setResourceItem(AbstractResourceItem* item)
{
    _manager->setResourceItem(item);
}
