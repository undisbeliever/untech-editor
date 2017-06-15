/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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
class Document;

class FrameSetDock : public QDockWidget {
    Q_OBJECT

public:
    explicit FrameSetDock(QWidget* parent = nullptr);
    ~FrameSetDock();

    void setDocument(Document* document);

    void clearGui();

private slots:
    void updateGui();
    void updateFrameListSelection();

    void onFrameListSelectionChanged();

private:
    std::unique_ptr<Ui::FrameSetDock> _ui;
    Document* _document;
};
}
}
}
}
