/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
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
class ResourceItem;
class PalettesModel;

class PalettesDock : public QDockWidget {
    Q_OBJECT

public:
    PalettesDock(QWidget* parent = nullptr);
    ~PalettesDock();

    void setResourceItem(ResourceItem* resourceItem);

private slots:
    void onSelectedPaletteChanged();
    void onPaletteListSelectionChanged();
    void onPaletteContextMenu(const QPoint& pos);

    void updateSelectedPalette();
    void updateSelectedColor();
    void onColorClicked(int colorIndex);

    void uncheckColorButtons();

private:
    std::unique_ptr<Ui::PalettesDock> const _ui;
    PalettesModel* const _model;
    Accessor::ListActions* const _listActions;

    ResourceItem* _resourceItem;

    QButtonGroup* const _colorGroup;
    QList<ColorToolButton*> const _colorButtons;
};
}
}
}
}
