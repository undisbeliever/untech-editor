/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include "models/metasprite/metasprite.h"
#include <QAction>
#include <QMenu>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace Animation {
class AnimationDock;
}

namespace SpriteImporter {
class ResourceItem;

class Actions : public QObject {
    Q_OBJECT

private:
    ResourceItem* _resourceItem;

public:
    QAction* const addRemoveTileHitbox;
    QAction* const toggleObjSize;
    QMenu* const entityHitboxTypeMenu;
    Accessor::NamedListActions* const frameListActions;
    Accessor::MultiListActions* const frameContentsActions;
    Accessor::NamedListActions* const animationListActions;
    Accessor::ListActions* const animationFrameActions;

public:
    Actions(Accessor::NamedListActions* frameListActions,
            Accessor::MultiListActions* frameContentsActions,
            Animation::AnimationDock* animationDock,
            QObject* parent = nullptr);
    ~Actions();

    void setResourceItem(ResourceItem* resourceItem);

    void populateEditMenu(QMenu* editMenu);
    void populateFrameContentsDockMenu(QMenu* menu);
    void populateGraphicsView(QWidget* widget);

private slots:
    void onFrameDataChanged(size_t frameIndex);

    void updateFrameActions();
    void updateFrameObjectActions();
    void updateEntityHitboxTypeMenu();

    void onAddRemoveTileHitbox();
    void onToggleObjSize();

    void onEntityHitboxTypeMenu(QAction* action);
};
}
}
}
}
