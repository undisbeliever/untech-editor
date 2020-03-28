/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "accessors.h"
#include "managers.h"
#include "resourceitem.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/entity/entity-function-tables/editorwidget.ui.h"
#include "models/entity/entityromdata-error.h"

using namespace UnTech::GuiQt::Entity::EntityFunctionTables;

EditorWidget::EditorWidget(QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
    , _manager(new EntityFunctionTablesManager(this))
    , _item(nullptr)
{
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

QString EditorWidget::windowStateName() const
{
    return QStringLiteral("EntityFunctionTables");
}

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<ResourceItem*>(abstractItem);

    _item = item;

    auto* ftList = item ? item->functionTableList() : nullptr;
    _manager->setFunctionTableList(ftList);

    setEnabled(item != nullptr);

    return item != nullptr;
}

void EditorWidget::onErrorDoubleClicked(const ErrorListItem& error)
{
    using namespace UnTech::Entity;

    if (_item == nullptr) {
        return;
    }

    if (const auto* e = dynamic_cast<const EntityFunctionTableError*>(error.specialized.get())) {
        _item->functionTableList()->setSelected_Ptr(e->ptr());
    }
}
