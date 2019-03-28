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
namespace Entity {
namespace EntityRomStructs {
namespace Ui {
class EntityRomStructListWidget;
}
class EntityRomStructsResourceItem;

class EntityRomStructListWidget : public QWidget {
    Q_OBJECT

public:
    explicit EntityRomStructListWidget(QWidget* parent = nullptr);
    ~EntityRomStructListWidget();

    void setResourceItem(EntityRomStructsResourceItem* item);

private:
    std::unique_ptr<Ui::EntityRomStructListWidget> const _ui;

    EntityRomStructsResourceItem* _item;
};

}
}
}
}
