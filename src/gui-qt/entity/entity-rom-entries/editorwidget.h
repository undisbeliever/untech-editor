/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QWidget>
#include <memory>

namespace UnTech {
class idstring;

namespace GuiQt {
namespace Entity {
namespace EntityRomEntries {
namespace Ui {
class EditorWidget;
}
class EntityRomEntriesResourceItem;
class EntityRomEntryManager;

class EditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit EditorWidget(QWidget* parent);
    ~EditorWidget();

    void setResourceItem(EntityRomEntriesResourceItem* item);

private:
    void clearGui();
    void updateFunctionTableComboList();

private slots:
    void onSelectedEntryChanged();

    void onEntryNameChanged(size_t index);
    void onEntryFunctionTableChanged(size_t index);
    void onEntryCommentChanged(size_t index);

    void onNameEdited();
    void onFunctionTableActivated();
    void onCommentEdited();

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;
    EntityRomEntryManager* const _manager;

    EntityRomEntriesResourceItem* _item;
};

}
}
}
}
