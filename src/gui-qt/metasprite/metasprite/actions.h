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
namespace MetaSprite {
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

    QAction* addPalette() const { return _addPalette; }
    QAction* clonePalette() const { return _clonePalette; }
    QAction* raisePalette() const { return _raisePalette; }
    QAction* lowerPalette() const { return _lowerPalette; }
    QAction* removePalette() const { return _removePalette; }

    QAction* addFrameObject() const { return _addFrameObject; }
    QAction* addActionPoint() const { return _addActionPoint; }
    QAction* addEntityHitbox() const { return _addEntityHitbox; }

    QAction* raiseSelected() const { return _raiseSelected; }
    QAction* lowerSelected() const { return _lowerSelected; }
    QAction* cloneSelected() const { return _cloneSelected; }
    QAction* removeSelected() const { return _removeSelected; }

    QAction* toggleObjSize() const { return _toggleObjSize; }
    QAction* flipObjHorizontally() const { return _flipObjHorizontally; }
    QAction* flipObjVertically() const { return _flipObjVertically; }

    QMenu* entityHitboxTypeMenu() const { return _entityHitboxTypeMenu.get(); }

public slots:
    void updateActions();

    void onAddFrame();
    void onCloneFrame();
    void onRenameFrame();
    void onRemoveFrame();

    void onAddRemoveTileHitbox();

    void onAddPalette();
    void onClonePalette();
    void onRemovePalette();
    void onRaisePalette();
    void onLowerPalette();

    void onAddFrameObject();
    void onAddActionPoint();
    void onAddEntityHitbox();

    void onRaiseSelected();
    void onLowerSelected();
    void onCloneSelected();
    void onRemoveSelected();

    void onToggleObjSize();
    void onFlipObjHorizontally();
    void onFlipObjVertically();

    void onEntityHitboxTypeMenu(QAction* action);

private:
    MainWindow* const _mainWindow;

    Document* _document;

    QAction* const _addFrame;
    QAction* const _cloneFrame;
    QAction* const _renameFrame;
    QAction* const _removeFrame;

    QAction* const _addRemoveTileHitbox;

    QAction* const _addPalette;
    QAction* const _clonePalette;
    QAction* const _raisePalette;
    QAction* const _lowerPalette;
    QAction* const _removePalette;

    QAction* const _addFrameObject;
    QAction* const _addActionPoint;
    QAction* const _addEntityHitbox;

    QAction* const _raiseSelected;
    QAction* const _lowerSelected;
    QAction* const _cloneSelected;
    QAction* const _removeSelected;

    QAction* const _toggleObjSize;
    QAction* const _flipObjHorizontally;
    QAction* const _flipObjVertically;

    std::unique_ptr<QMenu> const _entityHitboxTypeMenu;
};
}
}
}
}
