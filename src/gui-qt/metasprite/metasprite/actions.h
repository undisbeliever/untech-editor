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

    QAction* addRemoveTileHitbox() const { return _addRemoveTileHitbox; }

    QAction* toggleObjSize() const { return _toggleObjSize; }
    QAction* flipObjHorizontally() const { return _flipObjHorizontally; }
    QAction* flipObjVertically() const { return _flipObjVertically; }

    QMenu* entityHitboxTypeMenu() const { return _entityHitboxTypeMenu.get(); }

public slots:
    void updateSelectionActions();

    void onAddRemoveTileHitbox();

    void onToggleObjSize();
    void onFlipObjHorizontally();
    void onFlipObjVertically();

    void onEntityHitboxTypeMenu(QAction* action);

private:
    MainWindow* const _mainWindow;

    Document* _document;

    QAction* const _addRemoveTileHitbox;

    QAction* const _toggleObjSize;
    QAction* const _flipObjHorizontally;
    QAction* const _flipObjVertically;

    std::unique_ptr<QMenu> const _entityHitboxTypeMenu;
};
}
}
}
}
