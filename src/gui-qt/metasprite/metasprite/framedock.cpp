/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framedock.h"
#include "accessors.h"
#include "document.h"
#include "framecontentmanagers.h"
#include "gui-qt/accessor/idmaplistmodel.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/common/properties/propertytablemodel.h"
#include "gui-qt/metasprite/common.h"
#include "gui-qt/metasprite/metasprite/framedock.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

FrameDock::FrameDock(Accessor::IdmapListModel* frameListModel, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameDock)
    , _frameListModel(frameListModel)
    , _document(nullptr)
    , _frameObjectManager(new FrameObjectManager(this))
    , _actionPointManager(new ActionPointManager(this))
    , _entityHitboxManager(new EntityHitboxManager(this))
    , _addRemoveTileHitbox(new QAction(tr("Add Tile Hitbox"), this))
    , _toggleObjSize(new QAction(QIcon(":/icons/toggle-obj-size.svg"), tr("Toggle Object Size"), this))
    , _flipObjHorizontally(new QAction(QIcon(":/icons/flip-horizontally.svg"), tr("Flip Object Horizontally"), this))
    , _flipObjVertically(new QAction(QIcon(":/icons/flip-vertically.svg"), tr("Flip Object Vertically"), this))
    , _entityHitboxTypeMenu(new QMenu(tr("Set Entity Hitbox Type"), this))
{
    Q_ASSERT(frameListModel);

    _ui->setupUi(this);

    _ui->frameComboBox->setModel(_frameListModel);

    _ui->frameContents->setPropertyManagers(
        { _frameObjectManager, _actionPointManager, _entityHitboxManager },
        { tr("Location"), tr("Param"), tr("Tile"), tr("Flip") });

    _ui->frameContents->setIndentation(10);
    _ui->frameContents->header()->setStretchLastSection(true);
    _ui->frameContents->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto& frameActions = _ui->frameContents->viewActions();
    frameActions.add.at(0)->setIcon(QIcon(":/icons/add-frame-object.svg"));
    frameActions.add.at(1)->setIcon(QIcon(":/icons/add-action-point.svg"));
    frameActions.add.at(2)->setIcon(QIcon(":/icons/add-entity-hitbox.svg"));

    QMenu* frameContextMenu = _ui->frameContents->selectedContextmenu();
    QAction* firstAddAction = frameActions.add.first();
    frameContextMenu->insertAction(firstAddAction, _toggleObjSize);
    frameContextMenu->insertAction(firstAddAction, _flipObjHorizontally);
    frameContextMenu->insertAction(firstAddAction, _flipObjVertically);
    frameContextMenu->insertMenu(firstAddAction, _entityHitboxTypeMenu);
    frameContextMenu->insertSeparator(firstAddAction);

    populateEntityHitboxTypeMenu(_entityHitboxTypeMenu);

    _ui->frameContents->viewActions().populateToolbar(_ui->frameContentsButtons);

    clearGui();
    updateFrameActions();
    updateFrameObjectActions();
    updateEntityHitboxTypeMenu();

    setEnabled(false);

    connect(_ui->frameComboBox, qOverload<int>(&QComboBox::activated),
            this, &FrameDock::onFrameComboBoxActivated);

    connect(_ui->spriteOrder, &QSpinBox::editingFinished,
            this, &FrameDock::onSpriteOrderEdited);

    connect(_ui->solid, &QGroupBox::clicked,
            this, &FrameDock::onSolidClicked);
    connect(_ui->tileHitbox, &Ms8rectWidget::editingFinished,
            this, &FrameDock::onTileHitboxEdited);

    connect(_addRemoveTileHitbox, &QAction::triggered,
            this, &FrameDock::onAddRemoveTileHitbox);
    connect(_toggleObjSize, &QAction::triggered,
            this, &FrameDock::onToggleObjSize);
    connect(_flipObjHorizontally, &QAction::triggered,
            this, &FrameDock::onFlipObjHorizontally);
    connect(_flipObjVertically, &QAction::triggered,
            this, &FrameDock::onFlipObjVertically);
    connect(_entityHitboxTypeMenu, &QMenu::triggered,
            this, &FrameDock::onEntityHitboxTypeMenu);
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
        _document->entityHitboxList()->disconnect(this);
    }
    _document = document;

    _frameObjectManager->setDocument(_document);
    _actionPointManager->setDocument(_document);
    _entityHitboxManager->setDocument(_document);

    setEnabled(_document != nullptr);

    if (_document) {
        onSelectedFrameChanged();

        _ui->frameContents->setAccessors(
            _document->frameObjectList(),
            _document->actionPointList(),
            _document->entityHitboxList());

        connect(_document->frameMap(), &FrameMap::dataChanged,
                this, &FrameDock::onFrameDataChanged);

        connect(_document->frameMap(), &FrameMap::selectedItemChanged,
                this, &FrameDock::onSelectedFrameChanged);

        connect(_document->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &FrameDock::updateFrameObjectActions);

        connect(_document->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
                this, &FrameDock::updateEntityHitboxTypeMenu);
    }
    else {
        _ui->frameContents->setAccessors<
            FrameObjectList, ActionPointList, EntityHitboxList>(nullptr, nullptr, nullptr);

        clearGui();
    }
}

QMenu* FrameDock::frameContentsContextMenu() const
{
    return _ui->frameContents->selectedContextmenu();
}

void FrameDock::populateMenu(QMenu* editMenu)
{
    editMenu->addAction(_addRemoveTileHitbox);
    editMenu->addAction(_toggleObjSize);
    editMenu->addAction(_flipObjHorizontally);
    editMenu->addAction(_flipObjVertically);
    editMenu->addMenu(_entityHitboxTypeMenu);
    editMenu->addSeparator();
    _ui->frameContents->viewActions().populateMenu(editMenu, true);
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

    updateFrameActions();
}

void FrameDock::onFrameDataChanged(const void* frame)
{
    if (frame == _document->frameMap()->selectedFrame()) {
        updateGui();
        updateFrameActions();
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

void FrameDock::updateFrameActions()
{
    bool frameSelected = false;
    bool isFrameSolid = false;

    if (_document) {
        if (const auto* frame = _document->frameMap()->selectedFrame()) {
            frameSelected = true;
            isFrameSolid = frame->solid;
        }
    }

    _addRemoveTileHitbox->setEnabled(frameSelected);
    _addRemoveTileHitbox->setText(isFrameSolid ? tr("Remove Tile Hitbox")
                                               : tr("Add Tile Hitbox"));
}

void FrameDock::updateFrameObjectActions()
{
    bool objSelected = false;

    if (_document) {
        objSelected = !_document->frameObjectList()->selectedIndexes().empty();
    }

    _toggleObjSize->setEnabled(objSelected);
    _flipObjVertically->setEnabled(objSelected);
    _flipObjHorizontally->setEnabled(objSelected);
}

void FrameDock::updateEntityHitboxTypeMenu()
{
    bool ehSelected = false;

    if (_document) {
        ehSelected = !_document->entityHitboxList()->selectedIndexes().empty();
    }

    _entityHitboxTypeMenu->setEnabled(ehSelected);
}

void FrameDock::onSpriteOrderEdited()
{
    _document->frameMap()->editSelected_setSpriteOrder(
        _ui->spriteOrder->value());
}

void FrameDock::onSolidClicked()
{
    _document->frameMap()->editSelected_setSolid(
        _ui->solid->isChecked());
}

void FrameDock::onTileHitboxEdited()
{
    _document->frameMap()->editSelected_setTileHitbox(
        _ui->tileHitbox->valueMs8rect());
}

void FrameDock::onAddRemoveTileHitbox()
{
    _document->frameMap()->editSelected_toggleTileHitbox();
}

void FrameDock::onToggleObjSize()
{
    _document->frameObjectList()->editSelected_toggleObjectSize();
}

void FrameDock::onFlipObjHorizontally()
{
    _document->frameObjectList()->editSelected_flipObjectHorizontally();
}

void FrameDock::onFlipObjVertically()
{
    _document->frameObjectList()->editSelected_flipObjectVertically();
}

void FrameDock::onEntityHitboxTypeMenu(QAction* action)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;
    _document->entityHitboxList()->editSelected_setEntityHitboxType(
        EHT::from_romValue(action->data().toInt()));
}
