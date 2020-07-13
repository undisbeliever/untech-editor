/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "accessors.h"
#include "managers.h"
#include "resourceitem.h"
#include "gui-qt/common/properties/propertytablemodel.h"
#include "gui-qt/metatiles/interactive-tiles/editorwidget.ui.h"

using namespace UnTech::GuiQt::MetaTiles::InteractiveTiles;

EditorWidget::EditorWidget(QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
    , _manager(new FunctionTableManager(this))
    , _item(nullptr)
{
    _ui->setupUi(this);

    auto* fixedFunctionsManager = new FixedFunctionTableManager(this);
    auto* fixedFunctionsModel = new PropertyTableModel(fixedFunctionsManager, this);
    _ui->fixedFunctionsView->setModel(fixedFunctionsModel);
    _ui->fixedFunctionsView->setSelectionMode(QTreeView::NoSelection);
    _ui->fixedFunctionsView->setEditTriggers(QTreeView::NoEditTriggers);

    _ui->tableView->setPropertyManager(_manager);
    _ui->tableView->viewActions()->populate(_ui->tableButtons);

    setEnabled(false);
}

EditorWidget::~EditorWidget() = default;

QString EditorWidget::windowStateName() const
{
    return QStringLiteral("InteractiveTiles");
}

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<ResourceItem*>(abstractItem);

    if (_item == item) {
        return item != nullptr;
    }
    _item = item;

    auto* ftList = item ? item->functionTableList() : nullptr;
    _manager->setFunctionTableList(ftList);

    setEnabled(item != nullptr);

    return item != nullptr;
}

void EditorWidget::onErrorDoubleClicked(const ErrorListItem& error)
{
    if (_item == nullptr) {
        return;
    }

    if (const auto* e = dynamic_cast<const ListItemError*>(error.specialized.get())) {
        _item->functionTableList()->setSelected_Ptr(e->ptr());
    }
}
