/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityfunctiontableswidget.h"
#include "entityfunctiontablesresourceitem.h"
#include "managers.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/entity/entity-function-tables/entityfunctiontableswidget.ui.h"

using namespace UnTech::GuiQt::Entity;

EntityFunctionTablesWidget::EntityFunctionTablesWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::EntityFunctionTablesWidget>())
    , _manager(new EntityFunctionTablesManager(this))
{
    Q_ASSERT(parent);

    _ui->setupUi(this);

    _ui->tableView->setPropertyManager(_manager);
    _ui->tableView->viewActions()->populate(_ui->tableButtons);

    _ui->tableView->header()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    _ui->tableView->setColumnWidth(0, 180);
    _ui->tableView->setColumnWidth(1, 180);
    _ui->tableView->setColumnWidth(2, 180);

    setEnabled(false);
}

EntityFunctionTablesWidget::~EntityFunctionTablesWidget() = default;

void EntityFunctionTablesWidget::setResourceItem(EntityFunctionTablesResourceItem* item)
{
    auto* ftList = item ? item->functionTableList() : nullptr;
    _manager->setFunctionTableList(ftList);

    setEnabled(item != nullptr);
}
