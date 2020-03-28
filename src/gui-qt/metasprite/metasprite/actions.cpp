/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "actions.h"
#include "accessors.h"
#include "resourceitem.h"
#include "gui-qt/accessor/multilistactions.h"
#include "gui-qt/accessor/namedlistactions.h"
#include "gui-qt/metasprite/animation/animationdock.h"
#include "gui-qt/metasprite/common.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

Actions::Actions(Accessor::NamedListActions* fListActions,
                 Accessor::MultiListActions* fContentsActions,
                 Animation::AnimationDock* animationDock,
                 QObject* parent)
    : QObject(parent)
    , _resourceItem(nullptr)
    , addRemoveTileHitbox(new QAction(tr("Add Tile Hitbox"), this))
    , toggleObjSize(new QAction(QIcon(":/icons/toggle-obj-size.svg"), tr("Toggle Object Size"), this))
    , flipObjHorizontally(new QAction(QIcon(":/icons/flip-horizontally.svg"), tr("Flip Object Horizontally"), this))
    , flipObjVertically(new QAction(QIcon(":/icons/flip-vertically.svg"), tr("Flip Object Vertically"), this))
    , entityHitboxTypeMenu(new QMenu(tr("Set Entity Hitbox Type")))
    , frameListActions(fListActions)
    , frameContentsActions(fContentsActions)
    , animationListActions(animationDock->animationListActions())
    , animationFrameActions(animationDock->animationFrameActions())
{
    populateEntityHitboxTypeMenu(entityHitboxTypeMenu);

    updateFrameActions();
    updateFrameObjectActions();
    updateEntityHitboxTypeMenu();

    frameListActions->add->setShortcut(Qt::CTRL + Qt::Key_N);
    frameListActions->clone->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_D);

    Q_ASSERT(frameContentsActions->nAccessors() == 3);
    frameContentsActions->addAction(0)->setIcon(QIcon(":/icons/add-frame-object.svg"));
    frameContentsActions->addAction(0)->setShortcut(Qt::CTRL + Qt::Key_F);
    frameContentsActions->addAction(1)->setIcon(QIcon(":/icons/add-action-point.svg"));
    frameContentsActions->addAction(1)->setShortcut(Qt::CTRL + Qt::Key_P);
    frameContentsActions->addAction(2)->setIcon(QIcon(":/icons/add-entity-hitbox.svg"));
    frameContentsActions->addAction(2)->setShortcut(Qt::CTRL + Qt::Key_H);

    connect(addRemoveTileHitbox, &QAction::triggered,
            this, &Actions::onAddRemoveTileHitbox);
    connect(toggleObjSize, &QAction::triggered,
            this, &Actions::onToggleObjSize);
    connect(flipObjHorizontally, &QAction::triggered,
            this, &Actions::onFlipObjHorizontally);
    connect(flipObjVertically, &QAction::triggered,
            this, &Actions::onFlipObjVertically);
    connect(entityHitboxTypeMenu, &QMenu::triggered,
            this, &Actions::onEntityHitboxTypeMenu);
}

Actions::~Actions() = default;

void Actions::setResourceItem(ResourceItem* resourceItem)
{
    if (_resourceItem == resourceItem) {
        return;
    }

    if (_resourceItem != nullptr) {
        _resourceItem->frameList()->disconnect(this);
        _resourceItem->frameObjectList()->disconnect(this);
        _resourceItem->entityHitboxList()->disconnect(this);
    }
    _resourceItem = resourceItem;

    updateFrameActions();
    updateFrameObjectActions();
    updateEntityHitboxTypeMenu();

    if (_resourceItem) {
        connect(_resourceItem->frameList(), &FrameList::dataChanged,
                this, &Actions::onFrameDataChanged);

        connect(_resourceItem->frameList(), &FrameList::selectedIndexChanged,
                this, &Actions::updateFrameActions);

        connect(_resourceItem->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &Actions::updateFrameObjectActions);

        connect(_resourceItem->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
                this, &Actions::updateEntityHitboxTypeMenu);
    }
}

void Actions::populateEditMenu(QMenu* editMenu)
{
    editMenu->addSeparator();
    editMenu->addAction(frameListActions->add);
    frameContentsActions->populateAddActions(editMenu);
    editMenu->addSeparator();
    editMenu->addAction(addRemoveTileHitbox);
    editMenu->addAction(toggleObjSize);
    editMenu->addAction(flipObjHorizontally);
    editMenu->addAction(flipObjVertically);
    editMenu->addMenu(entityHitboxTypeMenu);
    editMenu->addSeparator();
    frameContentsActions->populateNotAddActions(editMenu);
    editMenu->insertAction(frameContentsActions->clone, frameListActions->clone);
}

void Actions::populateFrameContentsDockMenu(QMenu* menu)
{
    auto* firstAddAction = menu->actions().front();

    // Place these actions first as they have no shortcuts
    menu->insertAction(firstAddAction, toggleObjSize);
    menu->insertAction(firstAddAction, flipObjHorizontally);
    menu->insertAction(firstAddAction, flipObjVertically);
    menu->insertMenu(firstAddAction, entityHitboxTypeMenu);
    menu->insertSeparator(firstAddAction);
}

void Actions::populateGraphicsView(QWidget* widget)
{
    auto* menu = qobject_cast<QMenu*>(widget);
    const bool isMenu = menu != nullptr;

    // Place these actions first as they have no shortcuts
    widget->addAction(addRemoveTileHitbox);
    widget->addAction(toggleObjSize);
    widget->addAction(flipObjHorizontally);
    widget->addAction(flipObjVertically);

    if (menu) {
        menu->addMenu(entityHitboxTypeMenu);
        menu->addSeparator();
    }

    widget->addAction(frameListActions->add);
    frameContentsActions->populate(widget, isMenu);
    widget->insertAction(frameContentsActions->clone, frameListActions->clone);
}

void Actions::onFrameDataChanged(size_t frameIndex)
{
    if (frameIndex == _resourceItem->frameList()->selectedIndex()) {
        updateFrameActions();
    }
}

void Actions::updateFrameActions()
{
    bool frameSelected = false;
    bool isFrameSolid = false;

    if (_resourceItem) {
        if (const auto* frame = _resourceItem->frameList()->selectedItem()) {
            frameSelected = true;
            isFrameSolid = frame->solid;
        }
    }

    addRemoveTileHitbox->setEnabled(frameSelected);
    addRemoveTileHitbox->setText(isFrameSolid ? tr("Remove Tile Hitbox")
                                              : tr("Add Tile Hitbox"));
}

void Actions::updateFrameObjectActions()
{
    bool objSelected = false;

    if (_resourceItem) {
        objSelected = !_resourceItem->frameObjectList()->selectedIndexes().empty();
    }

    toggleObjSize->setEnabled(objSelected);
    flipObjVertically->setEnabled(objSelected);
    flipObjHorizontally->setEnabled(objSelected);
}

void Actions::updateEntityHitboxTypeMenu()
{
    bool ehSelected = false;

    if (_resourceItem) {
        ehSelected = !_resourceItem->entityHitboxList()->selectedIndexes().empty();
    }

    entityHitboxTypeMenu->setEnabled(ehSelected);
}

void Actions::onAddRemoveTileHitbox()
{
    _resourceItem->frameList()->editSelected_toggleTileHitbox();
}

void Actions::onToggleObjSize()
{
    _resourceItem->frameObjectList()->editSelected_toggleObjectSize();
}

void Actions::onFlipObjHorizontally()
{
    _resourceItem->frameObjectList()->editSelected_flipObjectHorizontally();
}

void Actions::onFlipObjVertically()
{
    _resourceItem->frameObjectList()->editSelected_flipObjectVertically();
}

void Actions::onEntityHitboxTypeMenu(QAction* action)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;
    _resourceItem->entityHitboxList()->editSelected_setEntityHitboxType(
        EHT::from_romValue(action->data().toInt()));
}
