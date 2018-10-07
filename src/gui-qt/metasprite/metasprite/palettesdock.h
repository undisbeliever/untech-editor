/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QButtonGroup>
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
class Document;
class PalettesModel;

class PalettesDock : public QDockWidget {
    Q_OBJECT

public:
    PalettesDock(QWidget* parent = nullptr);
    ~PalettesDock();

    void setDocument(Document* document);

private slots:
    void updateActions();
    void onSelectedPaletteChanged();
    void onPaletteListSelectionChanged();
    void onPaletteContextMenu(const QPoint& pos);

    void updateSelectedPalette();
    void updateSelectedColor();
    void onColorClicked(int colorIndex);

    void uncheckColorButtons();

    void onActionAdd();
    void onActionClone();
    void onActionRaise();
    void onActionLower();
    void onActionRemove();

private:
    std::unique_ptr<Ui::PalettesDock> const _ui;
    PalettesModel* const _model;

    Document* _document;

    QButtonGroup* const _colorGroup;
    QList<ColorToolButton*> const _colorButtons;
};
}
}
}
}
