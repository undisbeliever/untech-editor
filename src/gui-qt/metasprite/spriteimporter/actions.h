/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAction>
#include <QMenu>
#include <QObject>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class MainWindow;
class Document;

class Actions : public QObject {
    Q_OBJECT

public:
    Actions(MainWindow* mainWindow);
    ~Actions() = default;

    void setDocument(Document*);

    QAction* addRemoveTileHitbox() const { return _addRemoveTileHitbox; }

    QAction* addFrameObject() const { return _addFrameObject; }
    QAction* addActionPoint() const { return _addActionPoint; }
    QAction* addEntityHitbox() const { return _addEntityHitbox; }

    QAction* raiseSelected() const { return _raiseSelected; }
    QAction* lowerSelected() const { return _lowerSelected; }
    QAction* cloneSelected() const { return _cloneSelected; }
    QAction* removeSelected() const { return _removeSelected; }

    QAction* toggleObjSize() const { return _toggleObjSize; }
    QMenu* entityHitboxTypeMenu() const { return _entityHitboxTypeMenu.get(); }

public slots:
    void updateSelectionActions();

    void onAddRemoveTileHitbox();

    void onAddFrameObject();
    void onAddActionPoint();
    void onAddEntityHitbox();

    void onRaiseSelected();
    void onLowerSelected();
    void onCloneSelected();
    void onRemoveSelected();

    void onToggleObjSize();
    void onEntityHitboxTypeMenu(QAction* action);

private:
    MainWindow* const _mainWindow;

    Document* _document;

    QAction* const _addRemoveTileHitbox;

    QAction* const _addFrameObject;
    QAction* const _addActionPoint;
    QAction* const _addEntityHitbox;

    QAction* const _raiseSelected;
    QAction* const _lowerSelected;
    QAction* const _cloneSelected;
    QAction* const _removeSelected;

    QAction* const _toggleObjSize;
    std::unique_ptr<QMenu> const _entityHitboxTypeMenu;
};
}
}
}
}
