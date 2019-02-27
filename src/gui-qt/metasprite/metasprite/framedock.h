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
class FrameDock;
}
class Document;
class FrameManager;
class FrameObjectManager;
class ActionPointManager;
class EntityHitboxManager;

namespace MS = UnTech::MetaSprite::MetaSprite;

class FrameDock : public QDockWidget {
    Q_OBJECT

public:
    FrameDock(Accessor::NamedListModel* frameListModel,
              QWidget* parent = nullptr);
    ~FrameDock();

    void setDocument(Document* document);

    QMenu* frameContentsContextMenu() const;
    void populateMenu(QMenu* editMenu);

    void clearGui();

private slots:
    void onSelectedFrameChanged();
    void onFrameComboBoxActivated();

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
    std::unique_ptr<Ui::FrameDock> const _ui;
    Accessor::NamedListModel* const _frameListModel;

    Document* _document;

    FrameManager* const _frameManager;
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
