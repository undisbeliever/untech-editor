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
namespace MetaSprite {
namespace Ui {
class FrameSetDock;
}
class Actions;
class Document;

class FrameSetDock : public QDockWidget {
    Q_OBJECT

public:
    FrameSetDock(Actions* actions, QWidget* parent = nullptr);
    ~FrameSetDock();

    void setDocument(Document* document);

    void clearGui();

private slots:
    void updateGui();
    void updateFrameListSelection();

    void onNameEdited();
    void onTilesetTypeEdited();
    void onExportOrderButtonClicked();

    void onFrameListSelectionChanged();
    void onFrameContextMenu(const QPoint& pos);

private:
    std::unique_ptr<Ui::FrameSetDock> _ui;
    Actions* _actions;
    Document* _document;
};
}
}
}
}