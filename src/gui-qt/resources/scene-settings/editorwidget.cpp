/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "accessors.h"
#include "managers.h"
#include "resourceitem.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/resources/scene-settings/editorwidget.ui.h"

using namespace UnTech::GuiQt::Resources::SceneSettings;

EditorWidget::EditorWidget(QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
    , _manager(new SceneSettingsTableManager(this))
    , _item(nullptr)
{
    _ui->setupUi(this);

    _ui->tableView->setPropertyManager(_manager);
    _ui->tableView->viewActions()->populate(_ui->tableButtons);

    constexpr unsigned NAME_WIDTH = 200;
    constexpr unsigned COMBO_WIDTH = 160;

    _ui->tableView->header()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    _ui->tableView->setColumnWidth(0, NAME_WIDTH);
    _ui->tableView->setColumnWidth(1, COMBO_WIDTH);
    _ui->tableView->setColumnWidth(2, COMBO_WIDTH);
    _ui->tableView->setColumnWidth(3, COMBO_WIDTH);
    _ui->tableView->setColumnWidth(4, COMBO_WIDTH);
    _ui->tableView->setColumnWidth(5, COMBO_WIDTH);
    _ui->tableView->setColumnWidth(6, COMBO_WIDTH);

    setEnabled(false);
}

EditorWidget::~EditorWidget() = default;

QString EditorWidget::windowStateName() const
{
    return QStringLiteral("SceneSettings");
}

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<ResourceItem*>(abstractItem);

    _item = item;

    auto* ftList = item ? item->sceneSettingsList() : nullptr;
    _manager->setSceneSettingsList(ftList);

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
