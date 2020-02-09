/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilepropertieswidget.h"
#include "accessors.h"
#include "resourceitem.h"
#include "tilecollisionpixmaps.h"
#include "gui-qt/metatiles/mttileset/tilepropertieswidget.ui.h"
#include <QAbstractItemView>
#include <QButtonGroup>
#include <QListView>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;
using namespace UnTech::GuiQt::MetaTiles::MtTileset;

TilePropertiesWidget::TilePropertiesWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(new Ui::TilePropertiesWidget)
    , _collisionTypeButtons(new QButtonGroup(this))
    , _tileProperties(nullptr)
{
    using TC = MT::TileCollisionType;

    _ui->setupUi(this);

    const QColor iconColor = palette().color(QPalette::ButtonText);

    {
        // Create Collision Type Buttons
        _collisionTypeButtons->setExclusive(true);

        unsigned cell = 0;
        auto createButton = [&](const QString& toolTip, TC tc) {
            const static unsigned BUTTONS_PER_ROW = 6;

            auto* b = new QToolButton(this);
            b->setToolTip(toolTip);

            b->setCheckable(true);
            b->setIconSize(QSize(MT::METATILE_SIZE_PX, MT::METATILE_SIZE_PX));

            b->setIcon(TileCollisionTexture::createPixmap(tc, iconColor));

            _collisionTypeButtons->addButton(b, int(tc));

            _ui->tileCollisionTypeGrid->addWidget(b, cell / BUTTONS_PER_ROW, cell % BUTTONS_PER_ROW);
            cell++;
        };

        createButton(tr("No Collisions"), TC::EMPTY);
        createButton(tr("Solid"), TC::SOLID);
        createButton(tr("Up Platform"), TC::UP_PLATFORM);
        createButton(tr("Down Platform"), TC::DOWN_PLATFORM);
        createButton(tr("End Slope"), TC::END_SLOPE);
        cell++;

        createButton(tr("Down Right Slope"), TC::DOWN_RIGHT_SLOPE);
        createButton(tr("Down Left Slope"), TC::DOWN_LEFT_SLOPE);
        createButton(tr("Down Right Short Slope"), TC::DOWN_RIGHT_SHORT_SLOPE);
        createButton(tr("Down Right Tall Slope"), TC::DOWN_RIGHT_TALL_SLOPE);
        createButton(tr("Down Left Tall Slope"), TC::DOWN_LEFT_TALL_SLOPE);
        createButton(tr("Down Left Sort Slope"), TC::DOWN_LEFT_SHORT_SLOPE);

        createButton(tr("Up Right Slope"), TC::UP_RIGHT_SLOPE);
        createButton(tr("Up Left Slope"), TC::UP_LEFT_SLOPE);
        createButton(tr("Up Right Short Slope"), TC::UP_RIGHT_SHORT_SLOPE);
        createButton(tr("Up Right Tall Slope"), TC::UP_RIGHT_TALL_SLOPE);
        createButton(tr("Up Left Tall Slope"), TC::UP_LEFT_TALL_SLOPE);
        createButton(tr("Up Left Short Slope"), TC::UP_LEFT_SHORT_SLOPE);

        assert(_ui->tileCollisionTypeGrid->count() == MT::N_TILE_COLLISONS);
    }

    connect(_collisionTypeButtons, qOverload<int>(&QButtonGroup::buttonClicked),
            this, &TilePropertiesWidget::onCollisonTypeButtonClicked);
}

TilePropertiesWidget::~TilePropertiesWidget() = default;

void TilePropertiesWidget::setResourceItem(ResourceItem* item)
{
    auto* tp = item ? item->tileParameters() : nullptr;

    if (_tileProperties == tp) {
        return;
    }

    if (_tileProperties) {
        _tileProperties->disconnect(this);
    }
    _tileProperties = tp;

    if (_tileProperties) {
        connect(_tileProperties, &MtTilesetTileParameters::selectedIndexesChanged,
                this, &TilePropertiesWidget::updateGui);
        connect(_tileProperties, &MtTilesetTileParameters::tileCollisionsChanged,
                this, &TilePropertiesWidget::updateGui);
    }

    updateGui();
}

void TilePropertiesWidget::uncheckButtonGroup(QButtonGroup* group)
{
    if (auto* current = group->checkedButton()) {
        group->setExclusive(false);
        current->setChecked(false);
        group->setExclusive(true);
    }
}

void TilePropertiesWidget::checkButtonInGroup(QButtonGroup* group, int id)
{
    Q_ASSERT(group->exclusive());

    QAbstractButton* button = group->button(id);
    Q_ASSERT(button);
    button->setChecked(true);
}

void TilePropertiesWidget::updateGui()
{
    if (_tileProperties == nullptr
        || _tileProperties->selectedIndexes().empty()) {

        uncheckButtonGroup(_collisionTypeButtons);

        setEnabled(false);
    }
    else {
        auto tp = _tileProperties->selectedTileProperties();

        if (tp.tileCollisionSame) {
            checkButtonInGroup(_collisionTypeButtons, int(tp.tileCollision));
        }
        else {
            uncheckButtonGroup(_collisionTypeButtons);
        }

        setEnabled(true);
    }
}

void TilePropertiesWidget::onCollisonTypeButtonClicked(int buttonIndex)
{
    if (_tileProperties
        && buttonIndex >= 0 && buttonIndex < MT::N_TILE_COLLISONS) {

        _tileProperties->editSelectedTiles_setTileCollision(MT::TileCollisionType(buttonIndex));
    }
}