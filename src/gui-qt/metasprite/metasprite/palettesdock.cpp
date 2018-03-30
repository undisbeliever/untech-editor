/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettesdock.h"
#include "actions.h"
#include "document.h"
#include "palettecommands.h"
#include "palettesmodel.h"
#include "selection.h"
#include "gui-qt/common/widgets/colortoolbutton.h"
#include "gui-qt/metasprite/metasprite/palettesdock.ui.h"
#include "gui-qt/snes/snescolordialog.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;
using namespace UnTech::GuiQt::Snes;
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

PalettesDock::PalettesDock(Actions* actions, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::PalettesDock)
    , _actions(actions)
    , _model(new PalettesModel(this))
    , _document(nullptr)
    , _colorGroup(new QButtonGroup(this))
    , _colorButtons(buildColorButtons(_colorGroup, this))
{
    Q_ASSERT(actions != nullptr);

    _ui->setupUi(this);

    _ui->paletteList->setModel(_model);
    _ui->paletteList->setContextMenuPolicy(Qt::CustomContextMenu);

    _ui->paletteListButtons->addAction(_actions->addPalette());
    _ui->paletteListButtons->addAction(_actions->raisePalette());
    _ui->paletteListButtons->addAction(_actions->lowerPalette());
    _ui->paletteListButtons->addAction(_actions->clonePalette());
    _ui->paletteListButtons->addAction(_actions->removePalette());

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

    connect(_colorGroup, qOverload<int>(&QButtonGroup::buttonClicked),
            this, &PalettesDock::onColorClicked);

    connect(_ui->paletteList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PalettesDock::onPaletteListSelectionChanged);
}

PalettesDock::~PalettesDock() = default;

void PalettesDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    _model->setDocument(document);

    setEnabled(_document != nullptr);

    uncheckColorButtons();
    updateSelectedPalette();

    if (_document) {
        updatePaletteListSelection();
        updateSelectedColor();

        connect(_document, &Document::paletteChanged,
                this, &PalettesDock::updateSelectedPalette);
        connect(_document->selection(), &Selection::selectedPaletteChanged,
                this, &PalettesDock::updatePaletteListSelection);
        connect(_document->selection(), &Selection::selectedPaletteChanged,
                this, &PalettesDock::updateSelectedPalette);
        connect(_document->selection(), &Selection::selectedColorChanged,
                this, &PalettesDock::updateSelectedColor);
    }
}

void PalettesDock::updatePaletteListSelection()
{
    Q_ASSERT(_document);

    unsigned selectedPalette = _document->selection()->selectedPalette();
    QModelIndex index = _model->toModelIndex(selectedPalette);

    _ui->paletteList->setCurrentIndex(index);
}

void PalettesDock::onPaletteListSelectionChanged()
{
    if (_document) {
        QModelIndex index = _ui->paletteList->currentIndex();
        _document->selection()->selectPalette(index.row());
    }
}

void PalettesDock::onPaletteContextMenu(const QPoint& pos)
{
    if (_document && _actions) {
        bool onPalette = _ui->paletteList->indexAt(pos).isValid();

        QMenu menu;
        menu.addAction(_actions->addPalette());

        if (onPalette) {
            menu.addAction(_actions->clonePalette());
            menu.addAction(_actions->removePalette());
            menu.addSeparator();
            menu.addAction(_actions->raisePalette());
            menu.addAction(_actions->lowerPalette());
        }

        QPoint globalPos = _ui->paletteList->mapToGlobal(pos);
        menu.exec(globalPos);
    }
}

void PalettesDock::updateSelectedPalette()
{
    unsigned selectedPalette = INT_MAX;
    unsigned nPalettes = 0;

    if (_document) {
        selectedPalette = _document->selection()->selectedPalette();
        nPalettes = _document->frameSet()->palettes.size();
    }

    if (selectedPalette < nPalettes) {
        _ui->selectedPalette->setEnabled(true);

        const auto& palette = _document->frameSet()->palettes.at(selectedPalette);

        for (unsigned i = 0; i < 16; i++) {
            _colorButtons.at(i)->setColor(palette.color(i).rgb());
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
    const int c = _document->selection()->selectedColor();
    if (c >= 0) {
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

    if (_document) {
        _document->selection()->unselectColor();
    }
}

void PalettesDock::onColorClicked(int colorIndex)
{
    if (_document == nullptr) {
        return;
    }

    unsigned selectedPalette = _document->selection()->selectedPalette();
    const MS::FrameSet& frameSet = *_document->frameSet();

    if (selectedPalette >= frameSet.palettes.size()) {
        uncheckColorButtons();
    }
    else if (_ui->selectColorButton->isChecked()) {
        _document->selection()->selectColor(colorIndex);
    }
    else {
        editColorDialog(colorIndex);
    }
}

void PalettesDock::editColorDialog(int colorIndex)
{
    unsigned selectedPalette = _document->selection()->selectedPalette();
    const MS::FrameSet& frameSet = *_document->frameSet();

    const auto color = frameSet.palettes.at(selectedPalette).color(colorIndex);

    auto command = std::make_unique<ChangePaletteColor>(
        _document, selectedPalette, colorIndex);

    SnesColorDialog dialog(this);
    dialog.setColor(color);

    connect(&dialog, &SnesColorDialog::colorChanged,
            [&](auto& newColor) {
                command->setNewColor(newColor);
                command->redo();
            });

    dialog.exec();

    if (dialog.result() == QDialog::Accepted && dialog.color() != color) {
        _document->undoStack()->push(command.release());
    }
    else {
        command->undo();
    }

    uncheckColorButtons();
}
