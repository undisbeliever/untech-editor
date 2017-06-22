/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framedock.h"
#include "document.h"
#include "framecommands.h"
#include "framecontentsmodel.h"
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

    _ui->frameContents->header()->setStretchLastSection(true);
    _ui->frameContents->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    clearGui();
    setEnabled(false);

    connect(_ui->frameComboBox, SIGNAL(activated(int)),
            this, SLOT(onFrameComboBoxActivated()));

    connect(_ui->spriteOrder, SIGNAL(editingFinished()), this, SLOT(onSpriteOrderEdited()));

    connect(_ui->useGridLocation, SIGNAL(clicked(bool)), this, SLOT(onFrameLocationEdited()));
    connect(_ui->gridLocation, SIGNAL(editingFinished()), this, SLOT(onFrameLocationEdited()));
    connect(_ui->frameLocation, SIGNAL(editingFinished()), this, SLOT(onFrameLocationEdited()));
    connect(_ui->useCustomOrigin, SIGNAL(clicked(bool)), this, SLOT(onFrameLocationEdited()));
    connect(_ui->origin, SIGNAL(editingFinished()), this, SLOT(onFrameLocationEdited()));
    connect(_ui->solid, SIGNAL(clicked()), this, SLOT(onSolidClicked()));
    connect(_ui->tileHitbox, SIGNAL(editingFinished()), this, SLOT(onTileHitboxEdited()));
}

FrameDock::~FrameDock() = default;

void FrameDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    if (_document) {
        _ui->frameComboBox->setModel(_document->frameListModel());

        if (auto* m = _ui->frameContents->selectionModel()) {
            m->deleteLater();
        }
        _ui->frameContents->setModel(_document->frameContentsModel());

        onSelectedFrameChanged();

        connect(_document, SIGNAL(frameSetGridChanged()), this, SLOT(updateGui()));
        connect(_document, &Document::frameDataChanged, this, &FrameDock::onFrameDataChanged);

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
        _ui->frameContents->expandAll();

        _ui->frameComboBox->setCurrentIndex(
            _document->frameListModel()->toModelIndex(frameId).row());
    }
    else {
        clearGui();
    }
}

void FrameDock::onFrameDataChanged(const SI::Frame* frame)
{
    if (frame == _document->selection()->selectedFrame()) {
        updateGui();
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
    if (_document->selection()->selectedFrame() == nullptr) {
        return;
    }
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

void FrameDock::onSpriteOrderEdited()
{
    SI::Frame* frame = _document->selection()->selectedFrame();

    unsigned so = _ui->spriteOrder->value();
    if (so != frame->spriteOrder) {
        _document->undoStack()->push(
            new ChangeFrameSpriteOrder(_document, frame, so));
    }
}

void FrameDock::onFrameLocationEdited()
{
    const SI::FrameSet* frameSet = _document->frameSet();
    SI::Frame* frame = _document->selection()->selectedFrame();

    SI::FrameLocation floc;
    floc.useGridLocation = _ui->useGridLocation->isChecked();
    floc.gridLocation = _ui->gridLocation->valueUpoint();
    floc.aabb = _ui->frameLocation->valueUrect();
    floc.useGridOrigin = !_ui->useCustomOrigin->isChecked();
    floc.origin = _ui->origin->valueUpoint();

    floc.update(frameSet->grid, *frame);

    if (floc != frame->location) {
        _document->undoStack()->push(
            new ChangeFrameLocation(_document, frame, floc));
    }
}

void FrameDock::onSolidClicked()
{
    SI::Frame* frame = _document->selection()->selectedFrame();

    bool solid = _ui->solid->isChecked();
    if (solid != frame->solid) {
        _document->undoStack()->push(
            new ChangeFrameSolid(_document, frame, solid));
    }
}

void FrameDock::onTileHitboxEdited()
{
    SI::Frame* frame = _document->selection()->selectedFrame();

    urect hitbox = _ui->tileHitbox->valueUrect();
    if (hitbox != frame->tileHitbox) {
        _document->undoStack()->push(
            new ChangeFrameTileHitbox(_document, frame, hitbox));
    }
}
