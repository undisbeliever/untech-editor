/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityromentrylistwidget.h"
#include "accessors.h"
#include "entityromentriesresourceitem.h"
#include "gui-qt/entity/entity-rom-entries/entityromentrylistwidget.ui.h"

using namespace UnTech::GuiQt::Entity;

EntityRomEntryListWidget::EntityRomEntryListWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::EntityRomEntryListWidget>())
    , _item(nullptr)
{
    _ui->setupUi(this);

    _ui->listView->namedListActions()->populate(_ui->buttons);
}

EntityRomEntryListWidget::~EntityRomEntryListWidget() = default;

void EntityRomEntryListWidget::setResourceItem(EntityRomEntriesResourceItem* item)
{
    if (_item == item) {
        return;
    }
    _item = item;

    EntityRomEntriesList* entriesList = nullptr;
    if (item) {
        entriesList = item->entriesList();
    }
    _ui->listView->setAccessor(entriesList);
}
