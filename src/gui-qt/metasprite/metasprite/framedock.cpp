/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framedock.h"
#include "accessors.h"
#include "actions.h"
#include "document.h"
#include "framecontentmanagers.h"
#include "gui-qt/accessor/idmaplistmodel.h"
#include "gui-qt/accessor/idmapundohelper.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/common/properties/propertytablemodel.h"
#include "gui-qt/metasprite/metasprite/framedock.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

FrameDock::FrameDock(Accessor::IdmapListModel* frameListModel, Actions* actions, QWidget* parent)
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
          { tr("Location"), tr("Param"), QString("Tile"), QString("Flip") },
          this))
{
    Q_ASSERT(frameListModel);
    Q_ASSERT(actions != nullptr);

    _ui->setupUi(this);

    _ui->frameComboBox->setModel(_frameListModel);

    _ui->frameContents->setModel(_frameContentsModel);
    _ui->frameContents->setIndentation(10);
    _ui->frameContents->setEditTriggers(QAbstractItemView::AllEditTriggers);
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

    if (_document != nullptr) {
        _document->frameMap()->disconnect(this);
        _document->frameObjectList()->disconnect(this);
        _document->actionPointList()->disconnect(this);
        _document->entityHitboxList()->disconnect(this);
    }
    _document = document;

    _frameObjectManager->setDocument(_document);
    _actionPointManager->setDocument(_document);
    _entityHitboxManager->setDocument(_document);

    setEnabled(_document != nullptr);

    if (_document) {
        onSelectedFrameChanged();

        connect(_document->frameMap(), &FrameMap::dataChanged,
                this, &FrameDock::onFrameDataChanged);

        connect(_document->frameMap(), &FrameMap::selectedItemChanged,
                this, &FrameDock::onSelectedFrameChanged);

        connect(_document->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &FrameDock::updateFrameContentsSelection);

        connect(_document->actionPointList(), &ActionPointList::selectedIndexesChanged,
                this, &FrameDock::updateFrameContentsSelection);

        connect(_document->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
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
    const MS::Frame* frame = _document->frameMap()->selectedFrame();
    const idstring& frameId = _document->frameMap()->selectedId();

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
    if (frame == _document->frameMap()->selectedFrame()) {
        updateGui();
    }
}

void FrameDock::onFrameComboBoxActivated()
{
    int index = _ui->frameComboBox->currentIndex();

    if (index >= 0) {
        _document->frameMap()->setSelectedId(
            _frameListModel->toIdstring(index));
    }
    else {
        _document->frameMap()->unselectItem();
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
    if (_document->frameMap()->selectedFrame() == nullptr) {
        return;
    }
    const MS::Frame& frame = *_document->frameMap()->selectedFrame();

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
    using SOT = UnTech::MetaSprite::SpriteOrderType;

    SOT so = _ui->spriteOrder->value();

    FrameMapUndoHelper h(_document->frameMap());
    h.editSelectedItemField(so, tr("Edit Sprite Order"),
                            [](MS::Frame& f) -> SOT& { return f.spriteOrder; });
}

void FrameDock::onSolidClicked()
{
    bool solid = _ui->solid->isChecked();

    QString text = solid ? tr("Enable Tile Hitbox")
                         : tr("Disable Tile Hitbox");

    FrameMapUndoHelper h(_document->frameMap());
    h.editSelectedItemField(solid, text,
                            [](MS::Frame& f) -> bool& { return f.solid; });
}

void FrameDock::onTileHitboxEdited()
{
    ms8rect hitbox = _ui->tileHitbox->valueMs8rect();

    FrameMapUndoHelper h(_document->frameMap());
    h.editSelectedItemField(hitbox, tr("Edit Tile Hitbox"),
                            [](MS::Frame& f) -> ms8rect& { return f.tileHitbox; });
}

void FrameDock::updateFrameContentsSelection()
{
    QItemSelection sel;

    auto process = [&](const vectorset<size_t>& selectedIndexes,
                       PropertyTableManager* manager) {
        int managerIndex = _frameContentsModel->managers().indexOf(manager);

        for (size_t si : selectedIndexes) {
            QModelIndex index = _frameContentsModel->toModelIndex(managerIndex, si);
            if (index.isValid()) {
                sel.select(index, index);
            }
        }
    };

    process(_document->frameObjectList()->selectedIndexes(), _frameObjectManager);
    process(_document->actionPointList()->selectedIndexes(), _actionPointManager);
    process(_document->entityHitboxList()->selectedIndexes(), _entityHitboxManager);

    _ui->frameContents->selectionModel()->select(
        sel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void FrameDock::onFrameContentsSelectionChanged()
{
    std::vector<size_t> selectedObjects;
    std::vector<size_t> selectedActionPoints;
    std::vector<size_t> selectedEntityHitboxes;

    for (const auto& index : _ui->frameContents->selectionModel()->selectedRows()) {
        auto mi = _frameContentsModel->toManagerAndIndex(index);

        if (mi.first && mi.second >= 0) {
            if (mi.first == _frameObjectManager) {
                selectedObjects.push_back(mi.second);
            }
            else if (mi.first == _actionPointManager) {
                selectedActionPoints.push_back(mi.second);
            }
            else if (mi.first == _entityHitboxManager) {
                selectedEntityHitboxes.push_back(mi.second);
            }
        }
    }

    _document->frameObjectList()->setSelectedIndexes(std::move(selectedObjects));
    _document->actionPointList()->setSelectedIndexes(std::move(selectedActionPoints));
    _document->entityHitboxList()->setSelectedIndexes(std::move(selectedEntityHitboxes));
}

void FrameDock::onFrameContentsContextMenu(const QPoint& pos)
{
    if (_document && _actions) {
        QModelIndex modelIndex = _ui->frameContents->indexAt(pos);

        QMenu menu;
        bool addSep = false;
        if (_actions->toggleObjSize()->isEnabled()) {
            menu.addAction(_actions->toggleObjSize());
            menu.addAction(_actions->flipObjHorizontally());
            menu.addAction(_actions->flipObjVertically());
            addSep = true;
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
