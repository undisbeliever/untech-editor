/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framedock.h"
#include "actions.h"
#include "document.h"
#include "framecommands.h"
#include "framecontentsdelegate.h"
#include "framecontentsmodel.h"
#include "framelistmodel.h"
#include "selection.h"
#include "gui-qt/metasprite/metasprite/framedock.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

FrameDock::FrameDock(Actions* actions, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameDock)
    , _actions(actions)
    , _document(nullptr)
{
    Q_ASSERT(actions != nullptr);

    _ui->setupUi(this);

    _ui->frameContents->setContextMenuPolicy(Qt::CustomContextMenu);
    _ui->frameContents->setItemDelegate(new FrameContentsDelegate(this));
    _ui->frameContents->header()->setStretchLastSection(true);
    _ui->frameContents->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    clearGui();
    setEnabled(false);

    connect(_ui->frameComboBox, qOverload<int>(&QComboBox::activated),
            this, &FrameDock::onFrameComboBoxActivated);

    connect(_ui->spriteOrder, &QSpinBox::editingFinished,
            this, &FrameDock::onSpriteOrderEdited);

    connect(_ui->solid, &QGroupBox::clicked,
            this, &FrameDock::onSolidClicked);
    connect(_ui->tileHitbox, &Ms8rectWidget::editingFinished,
            this, &FrameDock::onTileHitboxEdited);

    connect(_ui->frameContents, &QTreeView::customContextMenuRequested,
            this, &FrameDock::onFrameContentsContextMenu);
}

FrameDock::~FrameDock() = default;

void FrameDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (auto* m = _ui->frameContents->selectionModel()) {
        m->deleteLater();
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    if (_document) {
        _ui->frameComboBox->setModel(_document->frameListModel());
        _ui->frameContents->setModel(_document->frameContentsModel());

        onSelectedFrameChanged();

        connect(_document, &Document::frameDataChanged, this, &FrameDock::onFrameDataChanged);

        connect(_document->selection(), &Selection::selectedFrameChanged,
                this, &FrameDock::onSelectedFrameChanged);

        connect(_document->selection(), &Selection::selectedItemsChanged,
                this, &FrameDock::updateFrameContentsSelection);

        connect(_ui->frameContents->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &FrameDock::onFrameContentsSelectionChanged);
    }
    else {
        _ui->frameComboBox->setModel(nullptr);
        _ui->frameContents->setModel(nullptr);

        clearGui();
    }
}

void FrameDock::onSelectedFrameChanged()
{
    MS::Frame* frame = _document->selection()->selectedFrame();
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

void FrameDock::onFrameDataChanged(const void* frame)
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
            _document->frameListModel()->toIdstring(index));
    }
    else {
        _document->selection()->unselectFrame();
    }
}

void FrameDock::clearGui()
{
    _ui->frameComboBox->setCurrentIndex(-1);

    _ui->spriteOrder->clear();

    _ui->solid->setChecked(false);
    _ui->tileHitbox->clear();
}

void FrameDock::updateGui()
{
    if (_document->selection()->selectedFrame() == nullptr) {
        return;
    }
    const MS::Frame& frame = *_document->selection()->selectedFrame();

    _ui->spriteOrder->setValue(frame.spriteOrder);

    _ui->solid->setChecked(frame.solid);

    if (frame.solid) {
        _ui->tileHitbox->setValue(frame.tileHitbox);
    }
    else {
        _ui->tileHitbox->clear();
    }
}

void FrameDock::onSpriteOrderEdited()
{
    MS::Frame* frame = _document->selection()->selectedFrame();

    unsigned so = _ui->spriteOrder->value();
    if (so != frame->spriteOrder) {
        _document->undoStack()->push(
            new ChangeFrameSpriteOrder(_document, frame, so));
    }
}

void FrameDock::onSolidClicked()
{
    MS::Frame* frame = _document->selection()->selectedFrame();

    bool solid = _ui->solid->isChecked();
    if (solid != frame->solid) {
        _document->undoStack()->push(
            new ChangeFrameSolid(_document, frame, solid));
    }
}

void FrameDock::onTileHitboxEdited()
{
    MS::Frame* frame = _document->selection()->selectedFrame();

    ms8rect hitbox = _ui->tileHitbox->valueMs8rect();
    if (hitbox != frame->tileHitbox) {
        _document->undoStack()->push(
            new ChangeFrameTileHitbox(_document, frame, hitbox));
    }
}

void FrameDock::updateFrameContentsSelection()
{
    FrameContentsModel* model = _document->frameContentsModel();
    QItemSelection sel;

    for (const auto& item : _document->selection()->selectedItems()) {
        QModelIndex index = model->toModelIndex(item);
        sel.select(index, index);
    }

    _ui->frameContents->selectionModel()->select(
        sel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void FrameDock::onFrameContentsSelectionChanged()
{
    FrameContentsModel* model = _document->frameContentsModel();
    std::set<SelectedItem> items;

    for (const auto& index : _ui->frameContents->selectionModel()->selectedRows()) {
        items.insert(model->toSelectedItem(index));
    }

    _document->selection()->setSelectedItems(items);
}

void FrameDock::onFrameContentsContextMenu(const QPoint& pos)
{
    if (_document && _actions) {
        QModelIndex modelIndex = _ui->frameContents->indexAt(pos);

        QMenu menu;
        menu.addAction(_actions->addFrameObject());
        menu.addAction(_actions->addActionPoint());
        menu.addAction(_actions->addEntityHitbox());

        if (modelIndex.isValid() && modelIndex.flags() & Qt::ItemIsEditable) {
            menu.addSeparator();
            menu.addAction(_actions->raiseSelected());
            menu.addAction(_actions->lowerSelected());
            menu.addAction(_actions->cloneSelected());
            menu.addAction(_actions->removeSelected());
        }

        QPoint globalPos = _ui->frameContents->mapToGlobal(pos);
        menu.exec(globalPos);
    }
}
