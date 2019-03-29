/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include "models/metasprite/metasprite.h"
#include <QDockWidget>
#include <QItemSelection>
#include <memory>

class QMenu;

namespace UnTech {
namespace GuiQt {
class PropertyTableModel;

namespace MetaSprite {
namespace MetaSprite {
namespace Ui {
class FrameContentsDock;
}
class Document;
class FrameObjectManager;
class ActionPointManager;
class EntityHitboxManager;

namespace MS = UnTech::MetaSprite::MetaSprite;

class FrameContentsDock : public QDockWidget {
    Q_OBJECT

public:
    FrameContentsDock(QWidget* parent = nullptr);
    ~FrameContentsDock();

    void setDocument(Document* document);

    QMenu* frameContentsContextMenu() const;
    void populateMenu(QMenu* editMenu);

private slots:
    void onSelectedFrameChanged();

    void onFrameDataChanged(size_t frameIndex);

    void updateFrameActions();
    void updateFrameObjectActions();
    void updateEntityHitboxTypeMenu();

    void onAddRemoveTileHitbox();
    void onToggleObjSize();
    void onFlipObjHorizontally();
    void onFlipObjVertically();

    void onEntityHitboxTypeMenu(QAction* action);

private:
    std::unique_ptr<Ui::FrameContentsDock> const _ui;

    Document* _document;

    FrameObjectManager* const _frameObjectManager;
    ActionPointManager* const _actionPointManager;
    EntityHitboxManager* const _entityHitboxManager;

    QAction* const _addRemoveTileHitbox;
    QAction* const _toggleObjSize;
    QAction* const _flipObjHorizontally;
    QAction* const _flipObjVertically;
    QMenu* const _entityHitboxTypeMenu;
};
}
}
}
}
