/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include <QAction>
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

    const Accessor::IdmapActions& frameActions() const;
    Accessor::IdmapListModel* frameListModel() const;

    void populateMenu(QMenu* menu);

    void clearGui();

private slots:
    void updateGui();

    void onNameEdited();
    void onTilesetTypeEdited();
    void onExportOrderEdited();

private:
    std::unique_ptr<Ui::FrameSetDock> const _ui;
    Actions* const _actions;

    Document* _document;
};
}
}
}
}
