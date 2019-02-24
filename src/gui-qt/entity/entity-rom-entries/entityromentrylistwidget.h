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
namespace Ui {
class EntityRomEntryListWidget;
}
class EntityRomEntriesResourceItem;

class EntityRomEntryListWidget : public QWidget {
    Q_OBJECT

public:
    explicit EntityRomEntryListWidget(QWidget* parent = nullptr);
    ~EntityRomEntryListWidget();

    void setResourceItem(EntityRomEntriesResourceItem* item);

private:
    std::unique_ptr<Ui::EntityRomEntryListWidget> const _ui;

    EntityRomEntriesResourceItem* _item;
};

}
}
}
