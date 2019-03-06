/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framedock.h"
#include "accessors.h"
#include "document.h"
#include "managers.h"
#include "gui-qt/accessor/namedlistmodel.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/common/properties/propertytablemodel.h"
#include "gui-qt/metasprite/common.h"
#include "gui-qt/metasprite/spriteimporter/framedock.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

FrameDock::FrameDock(Accessor::NamedListModel* frameListModel, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameDock)
    , _frameListModel(frameListModel)
    , _document(nullptr)
    , _frameManager(new FrameManager(this))
    , _frameObjectManager(new FrameObjectManager(this))
    , _actionPointManager(new ActionPointManager(this))
    , _entityHitboxManager(new EntityHitboxManager(this))
    , _addRemoveTileHitbox(new QAction(tr("Add Tile Hitbox"), this))
    , _toggleObjSize(new QAction(QIcon(":/icons/toggle-obj-size.svg"), tr("Toggle Object Size"), this))
    , _entityHitboxTypeMenu(new QMenu(tr("Set Entity Hitbox Type"), this))
{
    Q_ASSERT(frameListModel);

    _ui->setupUi(this);

    _ui->frameComboBox->setModel(_frameListModel);

    _ui->frameProperties->setPropertyManager(_frameManager);

    _ui->frameContents->setPropertyManagers(
        { _frameObjectManager, _actionPointManager, _entityHitboxManager },
        { tr("Location"), tr("Parameter") });

    _ui->frameContents->setIndentation(10);
    _ui->frameContents->header()->setStretchLastSection(true);
    _ui->frameContents->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto* frameActions = _ui->frameContents->viewActions();
    frameActions->addAction(0)->setIcon(QIcon(":/icons/add-frame-object.svg"));
    frameActions->addAction(1)->setIcon(QIcon(":/icons/add-action-point.svg"));
    frameActions->addAction(2)->setIcon(QIcon(":/icons/add-entity-hitbox.svg"));

    QMenu* frameContextMenu = _ui->frameContents->selectedContextmenu();
    QAction* firstAddAction = frameActions->addAction(0);
    frameContextMenu->insertAction(firstAddAction, _toggleObjSize);
    frameContextMenu->insertMenu(firstAddAction, _entityHitboxTypeMenu);
    frameContextMenu->insertSeparator(firstAddAction);

    populateEntityHitboxTypeMenu(_entityHitboxTypeMenu);

    _ui->frameContents->viewActions()->populateToolbar(_ui->frameContentsButtons);

    clearGui();
    updateFrameActions();
    updateFrameObjectActions();
    updateEntityHitboxTypeMenu();

    setEnabled(false);

    connect(_ui->frameComboBox, qOverload<int>(&QComboBox::activated),
            this, &FrameDock::onFrameComboBoxActivated);

    connect(_addRemoveTileHitbox, &QAction::triggered,
            this, &FrameDock::onAddRemoveTileHitbox);
    connect(_toggleObjSize, &QAction::triggered,
            this, &FrameDock::onToggleObjSize);
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
        _document->disconnect(this);
        _document->frameList()->disconnect(this);
        _document->frameObjectList()->disconnect(this);
        _document->entityHitboxList()->disconnect(this);
    }
    _document = document;

    _frameManager->setDocument(_document);
    _frameObjectManager->setDocument(_document);
    _actionPointManager->setDocument(_document);
    _entityHitboxManager->setDocument(_document);

    setEnabled(_document != nullptr);

    if (_document) {
        onSelectedFrameChanged();

        connect(_document->frameList(), &FrameList::dataChanged,
                this, &FrameDock::onFrameDataChanged);

        connect(_document->frameList(), &FrameList::selectedIndexChanged,
                this, &FrameDock::onSelectedFrameChanged);

        connect(_document->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &FrameDock::updateFrameObjectActions);

        connect(_document->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
                this, &FrameDock::updateEntityHitboxTypeMenu);
    }
    else {
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
    editMenu->addMenu(_entityHitboxTypeMenu);
    editMenu->addSeparator();
    _ui->frameContents->viewActions()->populateMenu(editMenu, true);
}

void FrameDock::onSelectedFrameChanged()
{
    const SI::Frame* frame = _document->frameList()->selectedItem();
    const size_t frameIndex = _document->frameList()->selectedIndex();

    if (frame) {
        _ui->frameContents->expandAll();

        _ui->frameComboBox->setCurrentIndex(
            _frameListModel->toModelIndex(frameIndex).row());
    }
    else {
        clearGui();
    }

    updateFrameActions();
}

void FrameDock::onFrameDataChanged(size_t frameIndex)
{
    if (frameIndex == _document->frameList()->selectedIndex()) {
        updateFrameActions();
    }
}

void FrameDock::onFrameComboBoxActivated()
{
    int index = _ui->frameComboBox->currentIndex();

    if (index >= 0) {
        _document->frameList()->setSelectedIndex(index);
    }
    else {
        _document->frameList()->unselectItem();
    }
}

void FrameDock::clearGui()
{
    _ui->frameComboBox->setCurrentIndex(-1);
}

void FrameDock::updateFrameActions()
{
    bool frameSelected = false;
    bool isFrameSolid = false;

    if (_document) {
        if (const auto* frame = _document->frameList()->selectedItem()) {
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
}

void FrameDock::updateEntityHitboxTypeMenu()
{
    bool ehSelected = false;

    if (_document) {
        ehSelected = !_document->entityHitboxList()->selectedIndexes().empty();
    }

    _entityHitboxTypeMenu->setEnabled(ehSelected);
}

void FrameDock::onAddRemoveTileHitbox()
{
    _document->frameList()->editSelected_toggleTileHitbox();
}

void FrameDock::onToggleObjSize()
{
    _document->frameObjectList()->editSelected_toggleObjectSize();
}

void FrameDock::onEntityHitboxTypeMenu(QAction* action)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;

    _document->entityHitboxList()->editSelected_setEntityHitboxType(
        EHT::from_romValue(action->data().toInt()));
}
