/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "accessors.h"
#include "managers.h"
#include "resourceitem.h"
#include "gui-qt/metasprite/actionpoints/editorwidget.ui.h"

using namespace UnTech::GuiQt::MetaSprite::ActionPoints;

EditorWidget::EditorWidget(QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
    , _manager(new ActionPointFunctionsManager(this))
    , _item(nullptr)
{
    _ui->setupUi(this);

    _ui->tableView->setPropertyManager(_manager);
    _ui->tableView->viewActions()->populate(_ui->tableButtons);

    _ui->tableView->header()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    _ui->tableView->setColumnWidth(0, 260);

    setEnabled(false);
}

EditorWidget::~EditorWidget() = default;

QString EditorWidget::windowStateName() const
{
    return QStringLiteral("ActionPoints");
}

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<ResourceItem*>(abstractItem);

    _item = item;

    auto* ftList = item ? item->actionPointFunctionsList() : nullptr;
    _manager->setAccessor(ftList);

    setEnabled(item != nullptr);

    return item != nullptr;
}

void EditorWidget::onErrorDoubleClicked(const UnTech::ErrorListItem& error)
{
    if (_item == nullptr) {
        return;
    }

    if (const auto* e = dynamic_cast<const ListItemError*>(error.specialized.get())) {
        _item->actionPointFunctionsList()->setSelected_Ptr(e->ptr());
    }
}
