/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "actions.h"
#include "accessors.h"
#include "document.h"
#include "mainwindow.h"
#include "gui-qt/accessor/idmapundohelper.h"
#include "gui-qt/accessor/listactionhelper.h"
#include "gui-qt/accessor/listandmultipleselectionundohelper.h"
#include "gui-qt/accessor/listundohelper.h"
#include "gui-qt/common/idstringdialog.h"

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

Actions::Actions(MainWindow* mainWindow)
    : QObject(mainWindow)
    , _mainWindow(mainWindow)
    , _document(nullptr)
    , _addRemoveTileHitbox(new QAction(tr("Add Tile Hitbox"), this))
    , _toggleObjSize(new QAction(QIcon(":/icons/toggle-obj-size.svg"), tr("Toggle Object Size"), this))
    , _flipObjHorizontally(new QAction(QIcon(":/icons/flip-horizontally.svg"), tr("Flip Object Horizontally"), this))
    , _flipObjVertically(new QAction(QIcon(":/icons/flip-vertically.svg"), tr("Flip Object Vertically"), this))
    , _entityHitboxTypeMenu(std::make_unique<QMenu>(tr("Set Entity Hitbox Type")))
{
    for (auto& it : UnTech::MetaSprite::EntityHitboxType::enumMap) {
        QString s = QString::fromStdString(it.first);
        _entityHitboxTypeMenu->addAction(s)->setData(int(it.second));
    }

    updateSelectionActions();

    connect(_addRemoveTileHitbox, &QAction::triggered, this, &Actions::onAddRemoveTileHitbox);

    connect(_toggleObjSize, &QAction::triggered, this, &Actions::onToggleObjSize);
    connect(_flipObjHorizontally, &QAction::triggered, this, &Actions::onFlipObjHorizontally);
    connect(_flipObjVertically, &QAction::triggered, this, &Actions::onFlipObjVertically);

    connect(_entityHitboxTypeMenu.get(), &QMenu::triggered, this, &Actions::onEntityHitboxTypeMenu);
}

void Actions::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
        _document->frameMap()->disconnect(this);
        _document->frameObjectList()->disconnect(this);
        _document->entityHitboxList()->disconnect(this);
    }
    _document = document;

    if (document) {
        connect(_document->frameMap(), &FrameMap::selectedItemChanged,
                this, &Actions::updateSelectionActions);
        connect(_document->frameObjectList(), &FrameObjectList::listChanged,
                this, &Actions::updateSelectionActions);
        connect(_document->entityHitboxList(), &EntityHitboxList::listChanged,
                this, &Actions::updateSelectionActions);
        connect(_document->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &Actions::updateSelectionActions);
        connect(_document->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
                this, &Actions::updateSelectionActions);
    }

    updateSelectionActions();
}

void Actions::updateSelectionActions()
{
    using namespace UnTech::GuiQt::Accessor;

    ListActionStatus obj;
    ListActionStatus eh;

    if (_document) {
        obj = ListActionHelper::status(_document->frameObjectList());
        eh = ListActionHelper::status(_document->entityHitboxList());
    }

    _toggleObjSize->setEnabled(obj.selectionValid);
    _flipObjVertically->setEnabled(obj.selectionValid);
    _flipObjHorizontally->setEnabled(obj.selectionValid);

    _entityHitboxTypeMenu->setEnabled(eh.selectionValid);
}

void Actions::onAddRemoveTileHitbox()
{
    const MS::Frame* frame = _document->frameMap()->selectedFrame();
    if (frame) {
        QString text = !frame->solid ? tr("Enable Tile Hitbox")
                                     : tr("Disable Tile Hitbox");

        FrameMapUndoHelper h(_document->frameMap());
        h.editSelectedItemField(!frame->solid, text,
                                [](MS::Frame& f) -> bool& { return f.solid; });
    }
}

void Actions::onToggleObjSize()
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    FrameObjectListUndoHelper h(_document->frameObjectList());
    h.editSelectedItems(tr("Change Object Size"),
                        [](MS::FrameObject& obj, size_t) {
                            obj.size = (obj.size == ObjSize::SMALL) ? ObjSize::LARGE : ObjSize::SMALL;
                        });
}

void Actions::onFlipObjHorizontally()
{
    FrameObjectListUndoHelper h(_document->frameObjectList());
    h.editSelectedItems(tr("Flip Horizontally"),
                        [](MS::FrameObject& obj, size_t) {
                            obj.hFlip = !obj.hFlip;
                        });
}

void Actions::onFlipObjVertically()
{
    FrameObjectListUndoHelper h(_document->frameObjectList());
    h.editSelectedItems(tr("Flip Vertically"),
                        [](MS::FrameObject& obj, size_t) {
                            obj.vFlip = !obj.vFlip;
                        });
}

void Actions::onEntityHitboxTypeMenu(QAction* action)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;

    EHT ehType = EHT::Enum(action->data().toInt());

    EntityHitboxListUndoHelper h(_document->entityHitboxList());
    h.setSelectedFields(ehType, tr("Change Entity Hitbox Type"),
                        [](MS::EntityHitbox& eh) -> EHT& { return eh.hitboxType; });
}
