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
namespace SpriteImporter {
namespace Ui {
class FrameSetDock;
}
class Document;
class FrameSetManager;

class FrameSetDock : public QDockWidget {
    Q_OBJECT

public:
    FrameSetDock(QWidget* parent = nullptr);
    ~FrameSetDock();

    void setDocument(Document* document);

    const Accessor::NamedListActions& frameActions() const;
    Accessor::NamedListModel* frameListModel() const;

    void populateMenu(QMenu* menu);

private:
    std::unique_ptr<Ui::FrameSetDock> const _ui;

    Document* _document;

    FrameSetManager* const _frameSetManager;
};
}
}
}
}
