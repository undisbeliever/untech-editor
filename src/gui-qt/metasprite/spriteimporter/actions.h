/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAction>
#include <QObject>

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
    QAction* removeFrame() const { return _removeFrame; }

public slots:
    void updateActions();

    void onAddFrame();
    void onCloneFrame();
    void onRemoveFrame();

private:
    MainWindow* _mainWindow;
    Document* _document;

    QAction* _addFrame;
    QAction* _cloneFrame;
    QAction* _removeFrame;
};
}
}
}
}
