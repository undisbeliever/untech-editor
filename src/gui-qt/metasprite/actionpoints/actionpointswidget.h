/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QWidget>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ActionPoints {
namespace Ui {
class ActionPointsWidget;
}
class ActionPointsResourceItem;
class ActionPointFunctionsManager;

class ActionPointsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ActionPointsWidget(QWidget* parent);
    ~ActionPointsWidget();

    void setResourceItem(ActionPointsResourceItem* item);

private:
    std::unique_ptr<Ui::ActionPointsWidget> const _ui;
    ActionPointFunctionsManager* const _manager;
};

}
}
}
}
