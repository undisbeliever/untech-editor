/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityromstructlistwidget.h"
#include "accessors.h"
#include "entityromstructsresourceitem.h"
#include "gui-qt/entity/entity-rom-structs/entityromstructlistwidget.ui.h"

using namespace UnTech::GuiQt::Entity;

EntityRomStructListWidget::EntityRomStructListWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::EntityRomStructListWidget>())
    , _item(nullptr)
{
    _ui->setupUi(this);

    _ui->listView->namedListActions().populate(_ui->buttons);
}

EntityRomStructListWidget::~EntityRomStructListWidget() = default;

void EntityRomStructListWidget::setResourceItem(EntityRomStructsResourceItem* item)
{
    if (_item == item) {
        return;
    }
    _item = item;

    EntityRomStructList* structList = nullptr;
    if (item) {
        structList = item->structList();
    }
    _ui->listView->setAccessor(structList);
}
