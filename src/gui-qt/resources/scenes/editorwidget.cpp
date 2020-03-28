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
#include "gui-qt/resources/scenes/editorwidget.ui.h"

using namespace UnTech::GuiQt::Resources::Scenes;

EditorWidget::EditorWidget(QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
    , _manager(new SceneTableManager(this))
    , _item(nullptr)
{
    _ui->setupUi(this);

    _ui->tableView->setPropertyManager(_manager);
    _ui->tableView->viewActions()->populate(_ui->tableButtons);

    constexpr unsigned NAME_WIDTH = 200;
    constexpr unsigned SETTINGS_WIDTH = 200;
    constexpr unsigned COMBO_WIDTH = 160;
    constexpr unsigned LAYER_TYPE_WIDTH = 35;

    _ui->tableView->header()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    _ui->tableView->setColumnWidth(0, NAME_WIDTH);
    _ui->tableView->setColumnWidth(1, SETTINGS_WIDTH);
    _ui->tableView->setColumnWidth(2, COMBO_WIDTH);
    _ui->tableView->setColumnWidth(3, LAYER_TYPE_WIDTH);
    _ui->tableView->setColumnWidth(4, COMBO_WIDTH);
    _ui->tableView->setColumnWidth(5, LAYER_TYPE_WIDTH);
    _ui->tableView->setColumnWidth(6, COMBO_WIDTH);
    _ui->tableView->setColumnWidth(7, LAYER_TYPE_WIDTH);
    _ui->tableView->setColumnWidth(8, COMBO_WIDTH);
    _ui->tableView->setColumnWidth(9, LAYER_TYPE_WIDTH);
    _ui->tableView->setColumnWidth(10, COMBO_WIDTH);

    setEnabled(false);
}

EditorWidget::~EditorWidget() = default;

QString EditorWidget::windowStateName() const
{
    return QStringLiteral("Scene");
}

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<ResourceItem*>(abstractItem);

    _item = item;

    auto* ftList = item ? item->sceneSettingsList() : nullptr;
    _manager->setSceneList(ftList);

    setEnabled(item != nullptr);

    return item != nullptr;
}

void EditorWidget::onErrorDoubleClicked(const ErrorListItem& error)
{
    if (_item == nullptr) {
        return;
    }

    if (const auto* e = dynamic_cast<const ListItemError*>(error.specialized.get())) {
        _item->sceneSettingsList()->setSelected_Ptr(e->ptr());
    }
}
