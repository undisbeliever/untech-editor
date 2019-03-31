/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstracteditorwidget.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace ProjectSettings {
namespace Ui {
class EditorWidget;
}
class ProjectSettingsResourceItem;
class ProjectSettingsPropertyManager;

class EditorWidget : public AbstractEditorWidget {
    Q_OBJECT

public:
    EditorWidget(QWidget* parent = nullptr);
    ~EditorWidget();

    virtual bool setResourceItem(AbstractResourceItem* abstractItem) final;

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;
    ProjectSettingsPropertyManager* const _manager;
};
}
}
}