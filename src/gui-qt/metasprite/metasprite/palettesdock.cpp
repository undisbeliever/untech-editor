/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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
#include <QSignalMapper>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;
using namespace UnTech::GuiQt::Snes;

PalettesDock::PalettesDock(Actions* actions, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::PalettesDock)
    , _actions(actions)
    , _document(nullptr)
{
    Q_ASSERT(actions != nullptr);

    _ui->setupUi(this);

    _ui->paletteList->setContextMenuPolicy(Qt::CustomContextMenu);

    for (unsigned i = 0; i < 16; i++) {
        ColorToolButton* b = new ColorToolButton(this);
        _colorButtons.append(b);

        // setting autoRaise removes gradient from QToolButton
        b->setAutoRaise(true);
        b->setIconSize(QSize(20, 20));

        _ui->colorGrid->addWidget(b, i / 8, i % 8);
    }

    setEnabled(false);

    connect(_ui->paletteList, &QListView::customContextMenuRequested,
            this, &PalettesDock::onPaletteContextMenu);

    QSignalMapper* signalMapper = new QSignalMapper(this);
    for (unsigned i = 0; i < 16; i++) {
        signalMapper->setMapping(_colorButtons.at(i), i);

        connect(_colorButtons.at(i), &QToolButton::clicked,
                signalMapper, qOverload<>(&QSignalMapper::map));
    }
    connect(signalMapper, qOverload<int>(&QSignalMapper::mapped),
            this, &PalettesDock::onColorClicked);
}

PalettesDock::~PalettesDock() = default;

void PalettesDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (auto* m = _ui->paletteList->selectionModel()) {
        m->deleteLater();
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    updateSelectedPalette();

    if (_document) {
        _ui->paletteList->setModel(_document->palettesModel());

        updatePaletteListSelection();

        connect(_document, &Document::paletteChanged,
                this, &PalettesDock::updateSelectedPalette);
        connect(_document->selection(), &Selection::selectedPaletteChanged,
                this, &PalettesDock::updatePaletteListSelection);
        connect(_document->selection(), &Selection::selectedPaletteChanged,
                this, &PalettesDock::updateSelectedPalette);
        connect(_ui->paletteList->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &PalettesDock::onPaletteListSelectionChanged);
    }
    else {
        _ui->paletteList->setModel(nullptr);
    }
}

void PalettesDock::updatePaletteListSelection()
{
    unsigned selectedPalette = _document->selection()->selectedPalette();
    QModelIndex index = _document->palettesModel()->toModelIndex(selectedPalette);

    _ui->paletteList->setCurrentIndex(index);
}

void PalettesDock::onPaletteListSelectionChanged()
{
    QModelIndex index = _ui->paletteList->currentIndex();
    _document->selection()->selectPalette(index.row());
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

void PalettesDock::onColorClicked(int colorIndex)
{
    if (_document == nullptr) {
        return;
    }

    unsigned selectedPalette = _document->selection()->selectedPalette();
    const MS::FrameSet& frameSet = *_document->frameSet();

    if (selectedPalette >= frameSet.palettes.size()) {
        return;
    }

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
}
