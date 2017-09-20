/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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

    QAction* addFrame() const { return _addFrame; }
    QAction* cloneFrame() const { return _cloneFrame; }
    QAction* renameFrame() const { return _renameFrame; }
    QAction* removeFrame() const { return _removeFrame; }

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
    void updateActions();

    void onAddFrame();
    void onCloneFrame();
    void onRenameFrame();
    void onRemoveFrame();

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
    MainWindow* _mainWindow;
    Document* _document;

    QAction* _addFrame;
    QAction* _cloneFrame;
    QAction* _renameFrame;
    QAction* _removeFrame;

    QAction* _addRemoveTileHitbox;

    QAction* _addFrameObject;
    QAction* _addActionPoint;
    QAction* _addEntityHitbox;

    QAction* _raiseSelected;
    QAction* _lowerSelected;
    QAction* _cloneSelected;
    QAction* _removeSelected;

    QAction* _toggleObjSize;
    std::unique_ptr<QMenu> _entityHitboxTypeMenu;
};
}
}
}
}
