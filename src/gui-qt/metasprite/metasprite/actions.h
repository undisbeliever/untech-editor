/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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
namespace MetaSprite {
class Document;

class Actions : public QObject {
    Q_OBJECT

private:
    Document* _document;

public:
    QAction* const addRemoveTileHitbox;
    QAction* const toggleObjSize;
    QAction* const flipObjHorizontally;
    QAction* const flipObjVertically;
    QMenu* const entityHitboxTypeMenu;
    Accessor::NamedListActions* const frameListActions;
    Accessor::MultiListActions* const frameContentsActions;

public:
    Actions(Accessor::NamedListActions* frameListActions,
            Accessor::MultiListActions* frameContentsActions,
            QObject* parent = nullptr);
    ~Actions();

    void setDocument(Document* document);

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
    void onFlipObjHorizontally();
    void onFlipObjVertically();

    void onEntityHitboxTypeMenu(QAction* action);
};
}
}
}
}
