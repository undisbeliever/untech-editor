/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QComboBox>
#include <QMainWindow>
#include <QUndoGroup>
#include <memory>

namespace UnTech {
namespace GuiQt {
class ZoomSettings;

namespace MetaSprite {
class LayerSettings;

namespace Animation {
class AnimationDock;
}
namespace MetaSprite {

namespace Ui {
class MainWindow;
}
class Document;
class Actions;
class MsGraphicsScene;
class FrameSetDock;
class FrameDock;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void setDocument(std::unique_ptr<Document> document);

protected:
    virtual void closeEvent(QCloseEvent* event);

private:
    void setupMenubar();
    void setupStatusbar();
    bool unsavedChangesDialog();

protected slots:
    void updateWindowTitle();

    void onActionNew();
    void onActionOpen();

    bool saveDocument();
    bool saveDocumentAs();

private:
    std::unique_ptr<Ui::MainWindow> _ui;
    std::unique_ptr<Document> _document;
    Actions* _actions;
    ZoomSettings* _zoomSettings;
    LayerSettings* _layerSettings;

    QUndoGroup* _undoGroup;

    MsGraphicsScene* _graphicsScene;
    FrameSetDock* _frameSetDock;
    FrameDock* _frameDock;
    Animation::AnimationDock* _animationDock;

    QComboBox* _zoomComboBox;
    QComboBox* _aspectRatioComboBox;
};
}
}
}
}
