/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstracteditorwidget.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ActionPoints {
namespace Ui {
class EditorWidget;
}
class ResourceItem;
class ActionPointFunctionsManager;

class EditorWidget : public AbstractEditorWidget {
    Q_OBJECT

public:
    explicit EditorWidget(QWidget* parent);
    ~EditorWidget();

    virtual bool setResourceItem(AbstractResourceItem* abstractItem) final;

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;
    ActionPointFunctionsManager* const _manager;
};

}
}
}
}
