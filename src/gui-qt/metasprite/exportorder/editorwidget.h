/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstracteditorwidget.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ExportOrder {
namespace Ui {
class EditorWidget;
}
class ExportOrderModel;
class ResourceItem;

class EditorWidget : public AbstractEditorWidget {
    Q_OBJECT

public:
    EditorWidget(QWidget* parent = nullptr);
    ~EditorWidget();

    virtual QString windowStateName() const final;

    virtual bool setResourceItem(AbstractResourceItem* abstractItem) final;

    virtual void onErrorDoubleClicked(const ErrorListItem& error) final;

private:
    void showEditorForCurrentIndex();
    void closeEditor();

private slots:
    void updateGui();
    void updateSelection();
    void updateActions();
    void onViewSelectionChanged();

    void onContextMenuRequested(const QPoint& pos);

    void onNameEdited();

    void onActionAddFrame();
    void onActionAddAnimation();
    void onActionAddAlternative();
    void onActionCloneSelected();
    void onActionRemoveSelected();

    void onActionRaiseToTop();
    void onActionRaise();
    void onActionLower();
    void onActionLowerToBottom();

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;
    ExportOrderModel* const _model;

    ResourceItem* _exportOrder;
};
}
}
}
}
