/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstracteditorwidget.h"
#include <memory>

namespace UnTech {
class idstring;

namespace GuiQt {
namespace Accessor {
class NamedListDock;
}
namespace Entity {
namespace EntityRomEntries {
namespace Ui {
class EditorWidget;
}
class ResourceItem;
class EntityRomEntryManager;

class EditorWidget : public AbstractEditorWidget {
    Q_OBJECT

public:
    explicit EditorWidget(QWidget* parent);
    ~EditorWidget();

    virtual QList<QDockWidget*> createDockWidgets(QMainWindow*) final;

    virtual bool setResourceItem(AbstractResourceItem* abstractItem) final;

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

    Accessor::NamedListDock* const _namedListDock;

    EntityRomEntryManager* const _manager;

    ResourceItem* _item;
};

}
}
}
}
