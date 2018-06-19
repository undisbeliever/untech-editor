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
#include "gui-qt/accessor/idmapundohelper.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/common/properties/propertytablemodel.h"
#include "gui-qt/metasprite/common.h"
#include "gui-qt/metasprite/spriteimporter/framedock.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

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
    , _entityHitboxTypeMenu(new QMenu(tr("Set Entity Hitbox Type"), this))
{
    Q_ASSERT(frameListModel);

    _ui->setupUi(this);

    _ui->frameComboBox->setModel(_frameListModel);

    _ui->frameContents->setPropertyManagers(
        { _frameObjectManager, _actionPointManager, _entityHitboxManager },
        { tr("Location"), tr("Parameter") });

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
    editMenu->addMenu(_entityHitboxTypeMenu);
    editMenu->addSeparator();
    _ui->frameContents->viewActions().populateMenu(editMenu, true);
}

void FrameDock::onSelectedFrameChanged()
{
    const SI::Frame* frame = _document->frameMap()->selectedFrame();
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
    if (_document->frameMap()->selectedFrame() == nullptr) {
        return;
    }
    const SI::Frame& frame = *_document->frameMap()->selectedFrame();
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
    using SOT = UnTech::MetaSprite::SpriteOrderType;

    SOT so = _ui->spriteOrder->value();

    FrameMapUndoHelper h(_document->frameMap());
    h.editSelectedItemField(so, tr("Edit Sprite Order"),
                            [](SI::Frame& f) -> SOT& { return f.spriteOrder; });
}

void FrameDock::onFrameLocationEdited()
{
    const SI::FrameSet* frameSet = _document->frameSet();
    const SI::Frame* frame = _document->frameMap()->selectedFrame();

    SI::FrameLocation floc;
    floc.useGridLocation = _ui->useGridLocation->isChecked();
    floc.gridLocation = _ui->gridLocation->valueUpoint();
    floc.aabb = _ui->frameLocation->valueUrect();
    floc.useGridOrigin = !_ui->useCustomOrigin->isChecked();
    floc.origin = _ui->origin->valueUpoint();

    floc.update(frameSet->grid, *frame);

    FrameMapUndoHelper h(_document->frameMap());
    h.editSelectedItemField(floc, tr("Edit Frame Location"),
                            [](SI::Frame& f) -> SI::FrameLocation& { return f.location; },
                            [](FrameMap* frameMap, const SI::Frame* f) { emit frameMap->frameLocationChanged(f); });
}

void FrameDock::onSolidClicked()
{
    bool solid = _ui->solid->isChecked();

    QString text = solid ? tr("Enable Tile Hitbox")
                         : tr("Disable Tile Hitbox");

    FrameMapUndoHelper h(_document->frameMap());
    h.editSelectedItemField(solid, text,
                            [](SI::Frame& f) -> bool& { return f.solid; });
}

void FrameDock::onTileHitboxEdited()
{
    urect hitbox = _ui->tileHitbox->valueUrect();

    FrameMapUndoHelper h(_document->frameMap());
    h.editSelectedItemField(hitbox, tr("Edit Tile Hitbox"),
                            [](SI::Frame& f) -> urect& { return f.tileHitbox; });
}

void FrameDock::onAddRemoveTileHitbox()
{
    const SI::Frame* frame = _document->frameMap()->selectedFrame();
    if (frame) {
        QString text = !frame->solid ? tr("Enable Tile Hitbox")
                                     : tr("Disable Tile Hitbox");

        FrameMapUndoHelper h(_document->frameMap());
        h.editSelectedItemField(!frame->solid, text,
                                [](SI::Frame& f) -> bool& { return f.solid; });
    }
}

void FrameDock::onToggleObjSize()
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    const SI::Frame* frame = _document->frameMap()->selectedFrame();
    Q_ASSERT(frame);

    FrameObjectListUndoHelper h(_document->frameObjectList());
    h.editSelectedItems(tr("Change Object Size"),
                        [&](SI::FrameObject& obj, size_t) {
                            obj.size = (obj.size == ObjSize::SMALL) ? ObjSize::LARGE : ObjSize::SMALL;

                            if (obj.bottomRight().x >= frame->location.aabb.width) {
                                obj.location.x = frame->location.aabb.width - obj.sizePx();
                            }
                            if (obj.bottomRight().y >= frame->location.aabb.height) {
                                obj.location.y = frame->location.aabb.height - obj.sizePx();
                            }
                        });
}

void FrameDock::onEntityHitboxTypeMenu(QAction* action)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;

    EHT eht = EHT::from_romValue(action->data().toInt());

    EntityHitboxListUndoHelper h(_document->entityHitboxList());
    h.setSelectedFields(eht, tr("Change Entity Hitbox Type"),
                        [](SI::EntityHitbox& eh) -> EHT& { return eh.hitboxType; });
}
