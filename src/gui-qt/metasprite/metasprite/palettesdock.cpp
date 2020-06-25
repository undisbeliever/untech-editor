/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettesdock.h"
#include "accessors.h"
#include "palettesmodel.h"
#include "resourceitem.h"
#include "gui-qt/accessor/listactions.h"
#include "gui-qt/common/widgets/colortoolbutton.h"
#include "gui-qt/metasprite/metasprite/palettesdock.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;
using ColorToolButton = UnTech::GuiQt::ColorToolButton;

static QList<ColorToolButton*> buildColorButtons(QButtonGroup* buttonGroup, PalettesDock* dock)
{
    QList<ColorToolButton*> buttons;

    for (unsigned i = 0; i < 16; i++) {
        ColorToolButton* b = new ColorToolButton(dock);
        buttons.append(b);
        buttonGroup->addButton(b, i);

        // setting autoRaise removes gradient from QToolButton
        b->setAutoRaise(true);
        b->setCheckable(true);
        b->setIconSize(QSize(16, 16));
    }

    return buttons;
}

PalettesDock::PalettesDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::PalettesDock)
    , _model(new PalettesModel(this))
    , _listActions(new Accessor::ListActions(this))
    , _resourceItem(nullptr)
    , _colorGroup(new QButtonGroup(this))
    , _colorButtons(buildColorButtons(_colorGroup, this))
{
    _ui->setupUi(this);

    _ui->paletteList->setModel(_model);
    _ui->paletteList->setContextMenuPolicy(Qt::CustomContextMenu);

    _listActions->populate(_ui->paletteListButtons);

    for (unsigned i = 0; i < 16; i++) {
        auto* b = _colorButtons.at(i);
        _ui->colorGrid->addWidget(b, i / 8, i % 8);
    }

    setEnabled(false);

    connect(_ui->paletteList, &QListView::customContextMenuRequested,
            this, &PalettesDock::onPaletteContextMenu);

    connect(_ui->editColorButton, &QToolButton::clicked,
            this, &PalettesDock::uncheckColorButtons);

    connect(_ui->selectColorButton, &QToolButton::clicked,
            this, &PalettesDock::uncheckColorButtons);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(_colorGroup, &QButtonGroup::idClicked,
            this, &PalettesDock::onColorClicked);
#else
    connect(_colorGroup, qOverload<int>(&QButtonGroup::buttonClicked),
            this, &PalettesDock::onColorClicked);
#endif

    connect(_ui->paletteList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PalettesDock::onPaletteListSelectionChanged);
}

PalettesDock::~PalettesDock() = default;

void PalettesDock::setResourceItem(ResourceItem* resourceItem)
{
    if (_resourceItem == resourceItem) {
        return;
    }

    if (_resourceItem != nullptr) {
        _resourceItem->disconnect(this);
        _resourceItem->paletteList()->disconnect(this);
    }
    _resourceItem = resourceItem;

    _model->setResourceItem(resourceItem);

    setEnabled(_resourceItem != nullptr);

    uncheckColorButtons();
    updateSelectedPalette();

    PaletteList* paletteList = nullptr;

    if (_resourceItem) {
        paletteList = _resourceItem->paletteList();

        onSelectedPaletteChanged();
        updateSelectedColor();

        connect(_resourceItem->paletteList(), &PaletteList::dataChanged,
                this, &PalettesDock::updateSelectedPalette);
        connect(_resourceItem->paletteList(), &PaletteList::selectedIndexChanged,
                this, &PalettesDock::onSelectedPaletteChanged);
        connect(_resourceItem->paletteList(), &PaletteList::selectedIndexChanged,
                this, &PalettesDock::updateSelectedPalette);
        connect(_resourceItem->paletteList(), &PaletteList::selectedColorChanged,
                this, &PalettesDock::updateSelectedColor);
    }

    _listActions->setAccessor(paletteList);
}

void PalettesDock::onSelectedPaletteChanged()
{
    Q_ASSERT(_resourceItem);

    unsigned selectedPalette = _resourceItem->paletteList()->selectedIndex();
    QModelIndex index = _model->toModelIndex(selectedPalette);

    _ui->paletteList->setCurrentIndex(index);
}

void PalettesDock::onPaletteListSelectionChanged()
{
    if (_resourceItem) {
        QModelIndex index = _ui->paletteList->currentIndex();
        if (index.isValid()) {
            _resourceItem->paletteList()->setSelectedIndex(index.row());
        }
        else {
            _resourceItem->paletteList()->unselectItem();
        }
    }
}

void PalettesDock::onPaletteContextMenu(const QPoint& pos)
{
    if (_resourceItem) {
        QMenu menu;
        _listActions->populate(&menu);

        QPoint globalPos = _ui->paletteList->mapToGlobal(pos);
        menu.exec(globalPos);
    }
}

void PalettesDock::updateSelectedPalette()
{
    const UnTech::Snes::Palette4bpp* palette = nullptr;

    if (_resourceItem) {
        palette = _resourceItem->paletteList()->selectedItem();
    }

    if (palette) {
        _ui->selectedPalette->setEnabled(true);

        for (unsigned i = 0; i < 16; i++) {
            _colorButtons.at(i)->setColor(palette->color(i).rgb());
        }
    }
    else {
        _ui->selectedPalette->setEnabled(false);

        for (auto* b : _colorButtons) {
            b->unsetColor();
        }
    }
}

void PalettesDock::updateSelectedColor()
{
    if (_resourceItem->paletteList()->isSelectedColorValid()) {
        const unsigned c = _resourceItem->paletteList()->selectedColor();

        _ui->selectColorButton->setChecked(true);
        _colorButtons.at(c)->setChecked(true);
    }
    else {
        uncheckColorButtons();
    }
}

void PalettesDock::uncheckColorButtons()
{
    Q_ASSERT(_colorGroup->exclusive());

    QAbstractButton* b = _colorGroup->checkedButton();

    if (b != nullptr) {
        _colorGroup->setExclusive(false);
        b->setChecked(false);
        _colorGroup->setExclusive(true);
    }

    if (_resourceItem) {
        _resourceItem->paletteList()->unselectColor();
    }
}

void PalettesDock::onColorClicked(int colorIndex)
{
    if (_resourceItem == nullptr) {
        return;
    }

    if (_resourceItem->paletteList()->isSelectedIndexValid() == false) {
        uncheckColorButtons();
    }
    else if (_ui->selectColorButton->isChecked()) {
        _resourceItem->paletteList()->setSelectedColor(colorIndex);
    }
    else {
        _resourceItem->paletteList()->editSelected_setColorDialog(colorIndex, this);
        uncheckColorButtons();
    }
}
