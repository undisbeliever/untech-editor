/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QDockWidget>

class QMenu;

namespace UnTech {
namespace GuiQt {
namespace Rooms {
namespace Ui {
class RoomEntitiesDock;
}
class ResourceItem;
class RoomEntitiesModel;

class RoomEntitiesDock final : public QDockWidget {
    Q_OBJECT

public:
    RoomEntitiesDock(QWidget* parent = nullptr);
    ~RoomEntitiesDock();

    void setResourceItem(ResourceItem* item);

    void populateActions(QWidget* widget) const;

private slots:
    void onAccessorSelectionChanged();
    void onViewSelectionChanged();

    void updateMoveToGroupMenuActions();

    void onSelectAllTriggered();

    void onAddEntityGroupTriggered();
    void onAddEntityEntryTriggered();
    void onCloneSelectedTriggered();
    void onRemoveSelectedTriggered();
    void onRaiseToTopTriggered();
    void onRaiseTriggered();
    void onLowerTriggered();
    void onLowerToBottomTriggered();

    void onMoveToGroupMenuTriggered(QAction* a);

    void onContextMenuRequested(const QPoint& pos);

private:
    void updateActions();

private:
    std::unique_ptr<Ui::RoomEntitiesDock> const _ui;
    RoomEntitiesModel* const _model;

    QMenu* const _moveToGroupMenu;

    ResourceItem* _resourceItem;
};

}
}
}
