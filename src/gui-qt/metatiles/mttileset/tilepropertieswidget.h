/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metatiles/metatile-tileset.h"
#include <QToolButton>
#include <QWidget>
#include <memory>

namespace UnTech {
struct ErrorListItem;

namespace GuiQt {
namespace MetaTiles {
namespace MtTileset {
namespace Ui {
class TilePropertiesWidget;
}
class ResourceItem;
class MtTilesetTileParameters;

namespace MT = UnTech::MetaTiles;

class TilePropertiesWidget : public QWidget {
    Q_OBJECT

public:
    explicit TilePropertiesWidget(QWidget* parent = nullptr);
    ~TilePropertiesWidget();

    void setResourceItem(ResourceItem* tileset);

private:
    static void uncheckButtonGroup(QButtonGroup* group);
    static void checkButtonInGroup(QButtonGroup* group, int id);

private slots:
    void updateGui();

    void onCollisonTypeButtonClicked(int buttonIndex);

private:
    std::unique_ptr<Ui::TilePropertiesWidget> const _ui;
    QButtonGroup* const _collisionTypeButtons;

    MtTilesetTileParameters* _tileProperties;
};

}
}
}
}
