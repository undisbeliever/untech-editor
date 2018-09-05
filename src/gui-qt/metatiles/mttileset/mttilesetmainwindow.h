/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QMainWindow>
#include <memory>

namespace UnTech {
namespace GuiQt {
class ZoomSettings;
class ZoomSettingsManager;

namespace MetaTiles {
namespace Ui {
class MtTilesetMainWindow;
}
class MtTilesetPropertyManager;
class MtTilesetResourceItem;

class MtTilesetMainWindow : public QMainWindow {
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
    explicit MtTilesetMainWindow(QWidget* parent, ZoomSettingsManager* zoomSettingsManager);
    ~MtTilesetMainWindow();

    void setResourceItem(MtTilesetResourceItem* item);

    ZoomSettings* zoomSettings() const;

private slots:
    void onTilesetStateChanged();

private:
    std::unique_ptr<Ui::MtTilesetMainWindow> const _ui;
    MtTilesetPropertyManager* const _propertyManager;

    MtTilesetResourceItem* _tileset;
};
}
}
}
