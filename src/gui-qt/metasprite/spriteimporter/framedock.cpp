/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framedock.h"
#include "document.h"
#include "framelistmodel.h"
#include "selection.h"
#include "gui-qt/metasprite/spriteimporter/framedock.ui.h"

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

FrameDock::FrameDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameDock)
    , _document(nullptr)
{
    _ui->setupUi(this);

    clearGui();
    setEnabled(false);

    connect(_ui->frameComboBox, SIGNAL(activated(int)),
            this, SLOT(onFrameComboBoxActivated()));
}

FrameDock::~FrameDock() = default;

void FrameDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->selection()->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    if (_document) {
        _ui->frameComboBox->setModel(_document->frameListModel());

        onSelectedFrameChanged();

        connect(_document->selection(), SIGNAL(selectedFrameChanged()),
                this, SLOT(onSelectedFrameChanged()));
    }
    else {
        clearGui();
    }
}

void FrameDock::onSelectedFrameChanged()
{
    SI::Frame* frame = _document->selection()->selectedFrame();
    const idstring& frameId = _document->selection()->selectedFrameId();

    _ui->frameWidget->setEnabled(frame != nullptr);
    _ui->frameContentsBox->setEnabled(frame != nullptr);

    if (frame) {
        updateGui();

        _ui->frameComboBox->setCurrentIndex(
            _document->frameListModel()->toModelIndex(frameId).row());
    }
    else {
        clearGui();
    }
}

void FrameDock::onFrameComboBoxActivated()
{
    int index = _ui->frameComboBox->currentIndex();

    if (index >= 0) {
        _document->selection()->selectFrame(
            _document->frameListModel()->toFrameId(index));
    }
    else {
        _document->selection()->unselectFrame();
    }
}

void FrameDock::clearGui()
{
    _ui->frameComboBox->setCurrentIndex(-1);

    _ui->gridLocation->setEnabled(false);
    _ui->frameLocation->setEnabled(false);
    _ui->origin->setEnabled(false);

    _ui->spriteOrder->clear();
    _ui->useGridLocation->setChecked(false);
    _ui->gridLocation->clear();
    _ui->frameLocation->clear();
    _ui->useCustomOrigin->setChecked(false);
    _ui->origin->clear();
    _ui->solid->setChecked(false);
    _ui->tileHitbox->clear();
}

void FrameDock::updateGui()
{
    const SI::Frame& frame = *_document->selection()->selectedFrame();
    const SI::FrameLocation& floc = frame.location;

    _ui->spriteOrder->setValue(frame.spriteOrder);

    _ui->useGridLocation->setChecked(floc.useGridLocation);
    _ui->gridLocation->setEnabled(floc.useGridLocation);
    _ui->gridLocation->setValue(floc.gridLocation);

    _ui->frameLocation->setEnabled(!floc.useGridLocation);
    _ui->frameLocation->setMinRectSize(frame.minimumViableSize());
    _ui->frameLocation->setValue(floc.aabb);

    _ui->useCustomOrigin->setChecked(!floc.useGridOrigin);
    _ui->origin->setEnabled(!floc.useGridOrigin);
    _ui->origin->setMaximum(floc.originRange());
    _ui->origin->setValue(floc.origin);

    _ui->solid->setChecked(frame.solid);

    if (frame.solid) {
        _ui->tileHitbox->setRange(frame.location.aabb.size());
        _ui->tileHitbox->setValue(frame.tileHitbox);
    }
    else {
        _ui->tileHitbox->clear();
    }
}
