/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecontentsdock.h"
#include "accessors.h"
#include "document.h"
#include "managers.h"
#include "gui-qt/accessor/namedlistmodel.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/common/properties/propertytablemodel.h"
#include "gui-qt/metasprite/common.h"
#include "gui-qt/metasprite/spriteimporter/framecontentsdock.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

FrameContentsDock::FrameContentsDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameContentsDock)
    , _document(nullptr)
    , _frameObjectManager(new FrameObjectManager(this))
    , _actionPointManager(new ActionPointManager(this))
    , _entityHitboxManager(new EntityHitboxManager(this))
    , _addRemoveTileHitbox(new QAction(tr("Add Tile Hitbox"), this))
    , _toggleObjSize(new QAction(QIcon(":/icons/toggle-obj-size.svg"), tr("Toggle Object Size"), this))
    , _entityHitboxTypeMenu(new QMenu(tr("Set Entity Hitbox Type"), this))
{
    _ui->setupUi(this);

    _ui->frameContents->setPropertyManagers(
        { _frameObjectManager, _actionPointManager, _entityHitboxManager },
        { tr("Location"), tr("Parameter") });

    _ui->frameContents->setIndentation(10);
    _ui->frameContents->header()->setStretchLastSection(true);
    _ui->frameContents->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto* frameActions = _ui->frameContents->viewActions();
    frameActions->addAction(0)->setIcon(QIcon(":/icons/add-frame-object.svg"));
    frameActions->addAction(0)->setShortcut(Qt::CTRL + Qt::Key_F);
    frameActions->addAction(1)->setIcon(QIcon(":/icons/add-action-point.svg"));
    frameActions->addAction(1)->setShortcut(Qt::CTRL + Qt::Key_P);
    frameActions->addAction(2)->setIcon(QIcon(":/icons/add-entity-hitbox.svg"));
    frameActions->addAction(2)->setShortcut(Qt::CTRL + Qt::Key_H);

    QMenu* frameContextMenu = _ui->frameContents->selectedContextmenu();
    QAction* firstAddAction = frameActions->addAction(0);
    frameContextMenu->insertAction(firstAddAction, _toggleObjSize);
    frameContextMenu->insertMenu(firstAddAction, _entityHitboxTypeMenu);
    frameContextMenu->insertSeparator(firstAddAction);

    populateEntityHitboxTypeMenu(_entityHitboxTypeMenu);

    _ui->frameContents->viewActions()->populate(_ui->frameContentsButtons);

    updateFrameActions();
    updateFrameObjectActions();
    updateEntityHitboxTypeMenu();

    setEnabled(false);

    connect(_addRemoveTileHitbox, &QAction::triggered,
            this, &FrameContentsDock::onAddRemoveTileHitbox);
    connect(_toggleObjSize, &QAction::triggered,
            this, &FrameContentsDock::onToggleObjSize);
    connect(_entityHitboxTypeMenu, &QMenu::triggered,
            this, &FrameContentsDock::onEntityHitboxTypeMenu);
}

FrameContentsDock::~FrameContentsDock() = default;

void FrameContentsDock::setDocument(Document* document)
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

    _frameObjectManager->setDocument(_document);
    _actionPointManager->setDocument(_document);
    _entityHitboxManager->setDocument(_document);

    setEnabled(_document != nullptr);

    if (_document) {
        onSelectedFrameChanged();

        connect(_document->frameList(), &FrameList::dataChanged,
                this, &FrameContentsDock::onFrameDataChanged);

        connect(_document->frameList(), &FrameList::selectedIndexChanged,
                this, &FrameContentsDock::onSelectedFrameChanged);

        connect(_document->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &FrameContentsDock::updateFrameObjectActions);

        connect(_document->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
                this, &FrameContentsDock::updateEntityHitboxTypeMenu);
    }
}

QMenu* FrameContentsDock::frameContentsContextMenu() const
{
    return _ui->frameContents->selectedContextmenu();
}

void FrameContentsDock::populateMenu(QMenu* editMenu)
{
    editMenu->addAction(_addRemoveTileHitbox);
    editMenu->addAction(_toggleObjSize);
    editMenu->addMenu(_entityHitboxTypeMenu);
    editMenu->addSeparator();
    _ui->frameContents->viewActions()->populate(editMenu, true);
}

void FrameContentsDock::onSelectedFrameChanged()
{
    const SI::Frame* frame = _document->frameList()->selectedItem();

    if (frame) {
        _ui->frameContents->expandAll();
    }

    updateFrameActions();
}

void FrameContentsDock::onFrameDataChanged(size_t frameIndex)
{
    if (frameIndex == _document->frameList()->selectedIndex()) {
        updateFrameActions();
    }
}

void FrameContentsDock::updateFrameActions()
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

void FrameContentsDock::updateFrameObjectActions()
{
    bool objSelected = false;

    if (_document) {
        objSelected = !_document->frameObjectList()->selectedIndexes().empty();
    }

    _toggleObjSize->setEnabled(objSelected);
}

void FrameContentsDock::updateEntityHitboxTypeMenu()
{
    bool ehSelected = false;

    if (_document) {
        ehSelected = !_document->entityHitboxList()->selectedIndexes().empty();
    }

    _entityHitboxTypeMenu->setEnabled(ehSelected);
}

void FrameContentsDock::onAddRemoveTileHitbox()
{
    _document->frameList()->editSelected_toggleTileHitbox();
}

void FrameContentsDock::onToggleObjSize()
{
    _document->frameObjectList()->editSelected_toggleObjectSize();
}

void FrameContentsDock::onEntityHitboxTypeMenu(QAction* action)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;

    _document->entityHitboxList()->editSelected_setEntityHitboxType(
        EHT::from_romValue(action->data().toInt()));
}
