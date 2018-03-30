/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framedock.h"
#include "actions.h"
#include "document.h"
#include "framecommands.h"
#include "framecontentmanagers.h"
#include "framelistmodel.h"
#include "selection.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/common/properties/propertytablemodel.h"
#include "gui-qt/metasprite/spriteimporter/framedock.ui.h"

#include <QMenu>

#include <QDebug>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

FrameDock::FrameDock(FrameListModel* frameListModel, Actions* actions, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameDock)
    , _frameListModel(frameListModel)
    , _actions(actions)
    , _document(nullptr)
    , _frameObjectManager(new FrameObjectManager(this))
    , _actionPointManager(new ActionPointManager(this))
    , _entityHitboxManager(new EntityHitboxManager(this))
    , _frameContentsModel(new PropertyTableModel(
          { _frameObjectManager, _actionPointManager, _entityHitboxManager },
          { tr("Location"), tr("Parameter") },
          this))
{
    Q_ASSERT(frameListModel);
    Q_ASSERT(actions != nullptr);

    _ui->setupUi(this);

    _ui->frameComboBox->setModel(_frameListModel);

    _ui->frameContents->setModel(_frameContentsModel);
    _ui->frameContents->setIndentation(10);
    _ui->frameContents->setContextMenuPolicy(Qt::CustomContextMenu);
    _ui->frameContents->setItemDelegate(new PropertyDelegate(this));
    _ui->frameContents->header()->setStretchLastSection(true);
    _ui->frameContents->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    _ui->frameContentsButtons->addAction(_actions->addFrameObject());
    _ui->frameContentsButtons->addAction(_actions->addActionPoint());
    _ui->frameContentsButtons->addAction(_actions->addEntityHitbox());
    _ui->frameContentsButtons->addAction(_actions->raiseSelected());
    _ui->frameContentsButtons->addAction(_actions->lowerSelected());
    _ui->frameContentsButtons->addAction(_actions->cloneSelected());
    _ui->frameContentsButtons->addAction(_actions->removeSelected());

    clearGui();
    setEnabled(false);

    connect(_ui->frameComboBox, qOverload<int>(&QComboBox::activated),
            this, &FrameDock::onFrameComboBoxActivated);

    connect(_ui->spriteOrder, &QSpinBox::editingFinished,
            this, &FrameDock::onSpriteOrderEdited);

    connect(_ui->useGridLocation, &QCheckBox::clicked,
            this, &FrameDock::onFrameLocationEdited);

    connect(_ui->gridLocation, &PointWidget::editingFinished,
            this, &FrameDock::onFrameLocationEdited);
    connect(_ui->frameLocation, &RectWidget::editingFinished,
            this, &FrameDock::onFrameLocationEdited);
    connect(_ui->useCustomOrigin, &QCheckBox::clicked,
            this, &FrameDock::onFrameLocationEdited);
    connect(_ui->origin, &PointWidget::editingFinished,
            this, &FrameDock::onFrameLocationEdited);

    connect(_ui->solid, &QGroupBox::clicked,
            this, &FrameDock::onSolidClicked);
    connect(_ui->tileHitbox, &RectWidget::editingFinished,
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

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    _frameObjectManager->setDocument(_document);
    _actionPointManager->setDocument(_document);
    _entityHitboxManager->setDocument(_document);

    setEnabled(_document != nullptr);

    if (_document) {
        onSelectedFrameChanged();

        connect(_document, &Document::frameSetGridChanged, this, &FrameDock::updateGui);
        connect(_document, &Document::frameDataChanged, this, &FrameDock::onFrameDataChanged);

        connect(_document->selection(), &Selection::selectedFrameChanged,
                this, &FrameDock::onSelectedFrameChanged);

        connect(_document->selection(), &Selection::selectedItemsChanged,
                this, &FrameDock::updateFrameContentsSelection);

        connect(_ui->frameContents->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &FrameDock::onFrameContentsSelectionChanged);
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
            _frameListModel->toModelIndex(frameId).row());
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
            _frameListModel->toIdstring(index));
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

void FrameDock::updateFrameContentsSelection()
{
    QItemSelection sel;

    for (const auto& item : _document->selection()->selectedItems()) {
        PropertyTableManager* manager = nullptr;
        if (item.type == SelectedItem::FRAME_OBJECT) {
            manager = _frameObjectManager;
        }
        else if (item.type == SelectedItem::ACTION_POINT) {
            manager = _actionPointManager;
        }
        else if (item.type == SelectedItem::ENTITY_HITBOX) {
            manager = _entityHitboxManager;
        }

        QModelIndex index = _frameContentsModel->toModelIndex(manager, item.index);
        if (index.isValid()) {
            sel.select(index, index);
        }
    }

    _ui->frameContents->selectionModel()->select(
        sel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void FrameDock::onFrameContentsSelectionChanged()
{
    std::set<SelectedItem> items;

    for (const auto& index : _ui->frameContents->selectionModel()->selectedRows()) {
        auto mi = _frameContentsModel->toManagerAndIndex(index);

        if (mi.first && mi.second >= 0) {
            unsigned i = unsigned(mi.second);

            if (mi.first == _frameObjectManager) {
                items.insert({ SelectedItem::FRAME_OBJECT, i });
            }
            else if (mi.first == _actionPointManager) {
                items.insert({ SelectedItem::ACTION_POINT, i });
            }
            else if (mi.first == _entityHitboxManager) {
                items.insert({ SelectedItem::ENTITY_HITBOX, i });
            }
        }
    }

    _document->selection()->setSelectedItems(items);
}

void FrameDock::onFrameContentsContextMenu(const QPoint& pos)
{
    if (_document && _actions) {
        QModelIndex modelIndex = _ui->frameContents->indexAt(pos);

        QMenu menu;
        bool addSep = false;
        if (_actions->toggleObjSize()->isEnabled()) {
            menu.addAction(_actions->toggleObjSize());
        }
        if (_actions->entityHitboxTypeMenu()->isEnabled()) {
            menu.addMenu(_actions->entityHitboxTypeMenu());
            addSep = true;
        }
        if (addSep) {
            menu.addSeparator();
        }
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
