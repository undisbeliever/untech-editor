/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QDockWidget>
#include <QList>
#include <memory>

namespace UnTech {
namespace GuiQt {
class ColorToolButton;

namespace MetaSprite {
namespace MetaSprite {
namespace Ui {
class PalettesDock;
}
class Actions;
class Document;

class PalettesDock : public QDockWidget {
    Q_OBJECT

public:
    PalettesDock(Actions* actions, QWidget* parent = nullptr);
    ~PalettesDock();

    void setDocument(Document* document);

private slots:
    void updatePaletteListSelection();
    void onPaletteListSelectionChanged();
    void onPaletteContextMenu(const QPoint& pos);

    void updateSelectedPalette();

private:
    std::unique_ptr<Ui::PalettesDock> _ui;
    Actions* _actions;
    Document* _document;

    QList<ColorToolButton*> _colorButtons;
};
}
}
}
}
