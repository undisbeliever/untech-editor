/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QMainWindow>
#include <memory>

class QGraphicsScene;

namespace UnTech {
struct ErrorListItem;

namespace GuiQt {
class ZoomSettings;
class ZoomSettingsManager;

namespace MetaTiles {
class Style;

namespace MtTileset {
namespace Ui {
class EditorWidget;
}
class MtTilesetPropertyManager;
class MtTilesetResourceItem;
class MtTilesetRenderer;
class MtTilesetGraphicsScene;
class MtEditableScratchpadGraphicsScene;
class MtScratchpadGraphicsScene;

class EditorWidget : public QMainWindow {
    Q_OBJECT

    enum StackIndex : int {
        ANIMATION_FRAMES_INPUT_WIDGET = 0,
        METATILES_CENTRAL_WIDGET = 1,
    };

    enum TabIndex : int {
        METATILES_TAB = 0,
        SCRATCHPAD_TAB = 1,
    };

public:
    explicit EditorWidget(QWidget* parent, ZoomSettingsManager* zoomSettingsManager);
    ~EditorWidget();

    void setResourceItem(MtTilesetResourceItem* item);

    ZoomSettings* zoomSettings() const;
    void populateMenu(QMenu* editMenu, QMenu* viewMenu);

    void onErrorDoubleClicked(const ErrorListItem& error);

private slots:
    void onTilesetStateChanged();

    void onTilesetPalettesChanged();
    void onPaletteComboActivated(const QString& paletteId);

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;
    Style* const _style;
    MtTilesetPropertyManager* const _tilesetPropertyManager;
    MtTilesetRenderer* const _renderer;
    MtTilesetGraphicsScene* const _tilesetScene;
    MtScratchpadGraphicsScene* const _scratchpadScene;
    MtEditableScratchpadGraphicsScene* const _editableScratchpadScene;

    MtTilesetResourceItem* _tileset;
};
}
}
}
}
