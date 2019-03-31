/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "entityfunctiontablesresourceitem.h"
#include "managers.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/entity/entity-function-tables/editorwidget.ui.h"

using namespace UnTech::GuiQt::Entity::EntityFunctionTables;

EditorWidget::EditorWidget(QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
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

EditorWidget::~EditorWidget() = default;

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<EntityFunctionTablesResourceItem*>(abstractItem);

    auto* ftList = item ? item->functionTableList() : nullptr;
    _manager->setFunctionTableList(ftList);

    setEnabled(item != nullptr);

    return item != nullptr;
}