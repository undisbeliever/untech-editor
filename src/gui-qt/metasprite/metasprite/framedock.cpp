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

using namespace UnTech::GuiQt;
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
{
    Q_ASSERT(frameListModel);
    Q_ASSERT(actions != nullptr);

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
    frameContextMenu->insertAction(firstAddAction, _actions->toggleObjSize());
    frameContextMenu->insertAction(firstAddAction, _actions->flipObjHorizontally());
    frameContextMenu->insertAction(firstAddAction, _actions->flipObjVertically());
    frameContextMenu->insertMenu(firstAddAction, _actions->entityHitboxTypeMenu());
    frameContextMenu->insertSeparator(firstAddAction);

    _ui->frameContents->viewActions().populateToolbar(_ui->frameContentsButtons);

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

        _ui->frameContents->setAccessors(
            _document->frameObjectList(),
            _document->actionPointList(),
            _document->entityHitboxList());

        connect(_document->frameMap(), &FrameMap::dataChanged,
                this, &FrameDock::onFrameDataChanged);

        connect(_document->frameMap(), &FrameMap::selectedItemChanged,
                this, &FrameDock::onSelectedFrameChanged);
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
    editMenu->addAction(_actions->toggleObjSize());
    editMenu->addAction(_actions->flipObjHorizontally());
    editMenu->addAction(_actions->flipObjVertically());
    editMenu->addMenu(_actions->entityHitboxTypeMenu());
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
