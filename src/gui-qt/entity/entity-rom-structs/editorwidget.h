/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/listactions.h"
#include <QWidget>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Entity {
namespace EntityRomStructs {
namespace Ui {
class EditorWidget;
}
class EntityRomStructsResourceItem;
class StructFieldsModel;

class EditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit EditorWidget(QWidget* parent);
    ~EditorWidget();

    void setResourceItem(EntityRomStructsResourceItem* item);

private:
    void clearGui();

private slots:
    void updateParentComboList();

    void onSelectedStructChanged();
    void onSelectedFieldsChanged();
    void onFieldViewSelectionChanged();

    void onStructNameChanged(size_t index);
    void onStructParentChanged(size_t index);
    void onStructCommentChanged(size_t index);

    void onNameEdited();
    void onParentActivated();
    void onCommentEdited();

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;
    StructFieldsModel* const _fieldsModel;

    Accessor::ListActions _fieldListActions;

    EntityRomStructsResourceItem* _item;
};

}
}
}
}
