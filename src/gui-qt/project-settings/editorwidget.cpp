/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "managers.h"
#include "resourceitem.h"
#include "gui-qt/project-settings/editorwidget.ui.h"

using namespace UnTech::GuiQt::ProjectSettings;

EditorWidget::EditorWidget(QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
    , _manager(new ProjectSettingsPropertyManager(this))
{
    _ui->setupUi(this);

    _ui->propertyView->setPropertyManager(_manager);
}

QString EditorWidget::windowStateName() const
{
    return QStringLiteral("ProjectSettings");
}

EditorWidget::~EditorWidget() = default;

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<ResourceItem*>(abstractItem);

    _manager->setResourceItem(item);

    return item != nullptr;
}
