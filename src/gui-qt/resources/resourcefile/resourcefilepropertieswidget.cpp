/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcefilepropertieswidget.h"
#include "resourcefilepropertymanager.h"
#include "gui-qt/resources/document.h"
#include "gui-qt/resources/resourcefile/resourcefilepropertieswidget.ui.h"

using namespace UnTech::GuiQt::Resources;

ResourceFilePropertiesWidget::ResourceFilePropertiesWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::ResourceFilePropertiesWidget>())
    , _manager(new ResourceFilePropertyManager(this))
{
    _ui->setupUi(this);

    _ui->propertyView->setPropertyManager(_manager);
}

ResourceFilePropertiesWidget::~ResourceFilePropertiesWidget() = default;

void ResourceFilePropertiesWidget::setDocument(Document* document)
{
    _manager->setDocument(document);
}
