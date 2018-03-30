/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QDockWidget>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
namespace Ui {
class FrameSetDock;
}
class FrameListModel;
class Actions;
class Document;

class FrameSetDock : public QDockWidget {
    Q_OBJECT

public:
    FrameSetDock(FrameListModel* frameListModel, Actions* actions,
                 QWidget* parent = nullptr);
    ~FrameSetDock();

    void setDocument(Document* document);

    void clearGui();

private slots:
    void updateGui();
    void updateFrameListSelection();

    void onNameEdited();
    void onTilesetTypeEdited();
    void onExportOrderFileSelected();
    void onImageFilenameFileSelected();
    void onTransparentColorSelected();
    void onGridEdited();
    void onPaletteEdited();

    void onFrameListSelectionChanged();
    void onFrameContextMenu(const QPoint& pos);

private:
    std::unique_ptr<Ui::FrameSetDock> const _ui;
    FrameListModel* const _frameListModel;
    Actions* const _actions;

    Document* _document;
};
}
}
}
}
