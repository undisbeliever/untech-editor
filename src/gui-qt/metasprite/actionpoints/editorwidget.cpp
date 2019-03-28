/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "actionpointsresourceitem.h"
#include "managers.h"
#include "gui-qt/metasprite/actionpoints/editorwidget.ui.h"

using namespace UnTech::GuiQt::MetaSprite::ActionPoints;

EditorWidget::EditorWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
    , _manager(new ActionPointFunctionsManager(this))
{
    Q_ASSERT(parent);

    _ui->setupUi(this);

    _ui->tableView->setPropertyManager(_manager);
    _ui->tableView->viewActions()->populate(_ui->tableButtons);

    _ui->tableView->header()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    _ui->tableView->setColumnWidth(0, 260);

    setEnabled(false);
}

EditorWidget::~EditorWidget() = default;

void EditorWidget::setResourceItem(ActionPointsResourceItem* item)
{
    auto* ftList = item ? item->actionPointFunctionsList() : nullptr;
    _manager->setAccessor(ftList);

    setEnabled(item != nullptr);
}
