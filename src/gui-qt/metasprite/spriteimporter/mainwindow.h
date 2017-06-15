/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QMainWindow>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {

namespace Ui {
class MainWindow;
}
class Document;
class FrameSetDock;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void setDocument(std::unique_ptr<Document> document);

protected slots:
    void onActionNew();
    void onActionOpen();
    void onActionSave();
    void onActionSaveAs();

private:
    std::unique_ptr<Ui::MainWindow> _ui;
    std::unique_ptr<Document> _document;

    FrameSetDock* _frameSetDock;
};
}
}
}
}
