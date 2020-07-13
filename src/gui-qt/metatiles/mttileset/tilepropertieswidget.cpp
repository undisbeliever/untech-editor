/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilepropertieswidget.h"
#include "accessors.h"
#include "resourceitem.h"
#include "tilecollisionpixmaps.h"
#include "gui-qt/metatiles/interactive-tiles/resourceitem.h"
#include "gui-qt/metatiles/mttileset/tilepropertieswidget.ui.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"
#include "models/metatiles/interactive-tiles.h"
#include "models/project/project.h"
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
    , _tilePriorityButtonGroup(new QButtonGroup(this))
    , _tilePriorityCheckBoxes{
        new QCheckBox(this),
        new QCheckBox(this),
        new QCheckBox(this),
        new QCheckBox(this),
    }
    , _tileProperties(nullptr)
    , _interactiveTiles(nullptr)
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

    {
        _tilePriorityButtonGroup->setExclusive(false);

        auto setupButton = [&](int id, const QString& text) {
            auto* cb = _tilePriorityCheckBoxes.at(id);
            cb->setText(text);
            cb->setTristate(true);

            _tilePriorityButtonGroup->addButton(cb, id);

            _ui->tilePriorityGrid->addWidget(cb, id / 2, id % 2);
        };

        setupButton(0, tr("Top Left"));
        setupButton(1, tr("Top Right"));
        setupButton(2, tr("Bottom Left"));
        setupButton(3, tr("Bottom Right"));
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(_collisionTypeButtons, &QButtonGroup::idClicked,
            this, &TilePropertiesWidget::onCollisonTypeButtonClicked);
#else
    connect(_collisionTypeButtons, qOverload<int>(&QButtonGroup::buttonClicked),
            this, &TilePropertiesWidget::onCollisonTypeButtonClicked);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(_tilePriorityButtonGroup, &QButtonGroup::idClicked,
            this, &TilePropertiesWidget::onTilePriorityClicked);
#else
    connect(_tilePriorityButtonGroup, qOverload<int>(&QButtonGroup::buttonClicked),
            this, &TilePropertiesWidget::onTilePriorityClicked);
#endif

    connect(_ui->functionTableCombo, qOverload<int>(&QComboBox::activated),
            this, &TilePropertiesWidget::onFunctionTableComboActivated);
}

TilePropertiesWidget::~TilePropertiesWidget() = default;

void TilePropertiesWidget::setResourceItem(ResourceItem* item)
{
    auto* tp = item ? item->tileParameters() : nullptr;
    auto* interactiveTiles = item ? item->project()->staticResources()->interactiveTiles() : nullptr;

    if (_tileProperties == tp) {
        Q_ASSERT(interactiveTiles == _interactiveTiles);
        return;
    }

    if (_tileProperties) {
        _tileProperties->disconnect(this);
    }
    _tileProperties = tp;

    if (_interactiveTiles) {
        _interactiveTiles->disconnect(this);
    }
    _interactiveTiles = interactiveTiles;

    if (_tileProperties) {
        connect(_tileProperties, &MtTilesetTileParameters::selectedIndexesChanged,
                this, &TilePropertiesWidget::updateGui);
        connect(_tileProperties, &MtTilesetTileParameters::tileCollisionsChanged,
                this, &TilePropertiesWidget::updateGui);
        connect(_tileProperties, &MtTilesetTileParameters::tileFunctionTablesChanged,
                this, &TilePropertiesWidget::updateGui);
        connect(_tileProperties, &MtTilesetTileParameters::tilePrioritiesChanged,
                this, &TilePropertiesWidget::updateGui);
    }

    if (_interactiveTiles) {
        connect(_interactiveTiles, &InteractiveTiles::ResourceItem::dataChanged,
                this, &TilePropertiesWidget::updateFunctionTableCombo);
    }

    updateFunctionTableCombo();
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

void TilePropertiesWidget::updateFunctionTableCombo()
{
    const auto* project = _interactiveTiles ? _interactiveTiles->project()->projectFile() : nullptr;

    if (project == nullptr) {
        _ui->functionTableCombo->clear();
    }
    else {
        auto addItems = [&](const auto& list) {
            for (const MT::InteractiveTileFunctionTable ft : list) {
                _ui->functionTableCombo->addItem(QString::fromStdString(ft.name));
            }
        };
        addItems(project->interactiveTiles.FIXED_FUNCTION_TABLES);
        addItems(project->interactiveTiles.functionTables);

        Q_ASSERT(_ui->functionTableCombo->itemText(0) == QStringLiteral("NoTileInteraction"));
        _ui->functionTableCombo->setItemText(0, QString());
    }
}

void TilePropertiesWidget::updateGui()
{
    if (_tileProperties == nullptr
        || _tileProperties->selectedIndexes().empty()) {

        uncheckButtonGroup(_collisionTypeButtons);
        uncheckButtonGroup(_tilePriorityButtonGroup);
        _ui->functionTableCombo->setCurrentText(QStringLiteral("---"));

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

        if (tp.functionTableSame) {
            _ui->functionTableCombo->setCurrentText(QString::fromStdString(tp.functionTable));
        }
        else {
            _ui->functionTableCombo->setCurrentText(QStringLiteral("---"));
        }

        const auto tilePriorities = _tileProperties->selectedTilePriorities();
        for (unsigned i = 0; i < tilePriorities.size(); i++) {
            _tilePriorityCheckBoxes.at(i)->setCheckState(tilePriorities.at(i));
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

void TilePropertiesWidget::onTilePriorityClicked(int subTileIndex)
{
    if (_tileProperties
        && subTileIndex >= 0 && subTileIndex < int(_tilePriorityCheckBoxes.size())) {

        bool b = _tilePriorityCheckBoxes.at(subTileIndex)->isChecked();
        _tileProperties->editSelectedTiles_setTilePriority(subTileIndex, b);
    }
}

void TilePropertiesWidget::onFunctionTableComboActivated()
{
    if (_tileProperties) {
        _tileProperties->editSelectedTiles_setFunctionTable(_ui->functionTableCombo->currentText().toStdString());
    }
}
