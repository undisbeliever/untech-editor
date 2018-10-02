/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "exportordereditorwidget.h"
#include "exportorderaccessors.h"
#include "exportordermodel.h"
#include "exportorderresourceitem.h"
#include "gui-qt/accessor/listactionhelper.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/metasprite/exportorder/exportordereditorwidget.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::ExportOrder;

ExportOrderEditorWidget::ExportOrderEditorWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::ExportOrderEditorWidget>())
    , _model(new ExportOrderModel(this))
    , _exportOrder(nullptr)
{
    _ui->setupUi(this);

    _ui->treeView->setModel(_model);

    _ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    _ui->treeView->setItemDelegate(new PropertyDelegate(this));
    _ui->treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    _ui->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _ui->treeView->setAlternatingRowColors(true);
    _ui->treeView->header()->hide();
    _ui->treeView->header()->setStretchLastSection(false);
    _ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    _ui->treeView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    _ui->treeView->header()->resizeSection(1, 65);

    connect(_ui->treeView, &QTreeView::customContextMenuRequested,
            this, &ExportOrderEditorWidget::onContextMenuRequested);

    connect(_ui->action_AddFrame, &QAction::triggered,
            this, &ExportOrderEditorWidget::onActionAddFrame);
    connect(_ui->action_AddAnimation, &QAction::triggered,
            this, &ExportOrderEditorWidget::onActionAddAnimation);
    connect(_ui->action_AddAlternative, &QAction::triggered,
            this, &ExportOrderEditorWidget::onActionAddAlternative);
    connect(_ui->action_CloneSelected, &QAction::triggered,
            this, &ExportOrderEditorWidget::onActionCloneSelected);
    connect(_ui->action_RemoveSelected, &QAction::triggered,
            this, &ExportOrderEditorWidget::onActionRemoveSelected);
    connect(_ui->action_RaiseToTop, &QAction::triggered,
            this, &ExportOrderEditorWidget::onActionRaiseToTop);
    connect(_ui->action_Raise, &QAction::triggered,
            this, &ExportOrderEditorWidget::onActionRaise);
    connect(_ui->action_Lower, &QAction::triggered,
            this, &ExportOrderEditorWidget::onActionLower);
    connect(_ui->action_LowerToBottom, &QAction::triggered,
            this, &ExportOrderEditorWidget::onActionLowerToBottom);
}

ExportOrderEditorWidget::~ExportOrderEditorWidget() = default;

void ExportOrderEditorWidget::setExportOrderResource(ExportOrderResourceItem* item)
{
    if (_exportOrder == item) {
        return;
    }

    if (_exportOrder) {
        _exportOrder->exportNameList()->disconnect(this);
        _exportOrder->alternativesList()->disconnect(this);

        // required to prevent selectionModel from clearing resource selection
        _ui->treeView->selectionModel()->disconnect(this);
    }
    _exportOrder = item;

    _model->setExportOrder(item);
    _ui->treeView->expandAll();

    this->setEnabled(item != nullptr && item->exportOrder() != nullptr);

    if (_exportOrder) {
        updateSelection();
        updateActions();

        // prevents data corruption when the list changes and the editor is open
        connect(_exportOrder->exportNameList(), &ExportOrder::ExportNameList::listAboutToChange,
                this, &ExportOrderEditorWidget::closeEditor);
        connect(_exportOrder->alternativesList(), &ExportOrder::AlternativesList::listAboutToChange,
                this, &ExportOrderEditorWidget::closeEditor);

        connect(_exportOrder->exportNameList(), &ExportOrder::ExportNameList::selectedIndexChanged,
                this, &ExportOrderEditorWidget::updateSelection);
        connect(_exportOrder->alternativesList(), &ExportOrder::AlternativesList::selectedIndexChanged,
                this, &ExportOrderEditorWidget::updateSelection);

        connect(_exportOrder->exportNameList(), &ExportOrder::ExportNameList::listChanged,
                this, &ExportOrderEditorWidget::updateActions);
        connect(_exportOrder->alternativesList(), &ExportOrder::AlternativesList::listChanged,
                this, &ExportOrderEditorWidget::updateActions);
        connect(_exportOrder->exportNameList(), &ExportOrder::ExportNameList::selectedIndexChanged,
                this, &ExportOrderEditorWidget::updateActions);
        connect(_exportOrder->alternativesList(), &ExportOrder::AlternativesList::selectedIndexChanged,
                this, &ExportOrderEditorWidget::updateActions);

        connect(_ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &ExportOrderEditorWidget::onViewSelectionChanged);
    }
}

void ExportOrderEditorWidget::updateSelection()
{
    using InternalIdFormat = ExportOrderModel::InternalIdFormat;

    Q_ASSERT(_exportOrder);

    if (_exportOrder->exportNameList()->isSelectedItemValid()) {
        InternalIdFormat id;
        id.isFrame = _exportOrder->exportNameList()->selectedListIsFrame();
        id.index = _exportOrder->exportNameList()->selectedIndex();
        id.altIndex = _exportOrder->alternativesList()->selectedIndex();

        QModelIndex index = _model->toModelIndex(id);
        _ui->treeView->expand(index);

        _ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        _ui->treeView->setCurrentIndex(index);
        _ui->treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    }
    else {
        _ui->treeView->setCurrentIndex(QModelIndex());
    }
}

void ExportOrderEditorWidget::updateActions()
{
    using namespace UnTech::GuiQt::Accessor;

    Q_ASSERT(_exportOrder);

    auto* enList = _exportOrder->exportNameList();
    auto* altList = _exportOrder->alternativesList();

    ListActionStatus enStatus = ListActionHelper::status(enList);
    ListActionStatus altStatus = ListActionHelper::status(altList);

    ListActionStatus selStatus = altStatus.selectionValid ? altStatus : enStatus;

    _ui->action_AddFrame->setEnabled(ListActionHelper::canAddToList(enList, true));
    _ui->action_AddAnimation->setEnabled(ListActionHelper::canAddToList(enList, false));
    _ui->action_AddAlternative->setEnabled(altStatus.canAdd);
    _ui->action_RaiseToTop->setEnabled(selStatus.canRaise);
    _ui->action_Raise->setEnabled(selStatus.canRaise);
    _ui->action_Lower->setEnabled(selStatus.canLower);
    _ui->action_LowerToBottom->setEnabled(selStatus.canLower);
    _ui->action_CloneSelected->setEnabled(selStatus.canClone);
    _ui->action_RemoveSelected->setEnabled(selStatus.canRemove);
}

void ExportOrderEditorWidget::onViewSelectionChanged()
{
    using InternalIdFormat = ExportOrderModel::InternalIdFormat;

    if (_exportOrder == nullptr) {
        return;
    }

    QModelIndex index = _ui->treeView->currentIndex();
    if (index.isValid()) {
        InternalIdFormat id = index.internalId();
        _exportOrder->exportNameList()->setSelectedListIsFrame(id.isFrame);
        _exportOrder->exportNameList()->setSelectedIndex(id.index);
        _exportOrder->alternativesList()->setSelectedIndex(id.altIndex);
    }
    else {
        _exportOrder->exportNameList()->unselectItem();
    }
}

void ExportOrderEditorWidget::onContextMenuRequested(const QPoint& pos)
{
    using InternalIdFormat = ExportOrderModel::InternalIdFormat;

    if (_exportOrder && _exportOrder->exportOrder()) {
        QMenu menu;

        QModelIndex index = _ui->treeView->currentIndex();
        if (index.isValid()) {
            InternalIdFormat intId = index.internalId();

            if (intId.isFrame) {
                menu.addAction(_ui->action_AddFrame);
            }
            else {
                menu.addAction(_ui->action_AddAnimation);
            }

            if (intId.index != InternalIdFormat::NO_INDEX) {
                menu.addAction(_ui->action_AddAlternative);
                menu.addAction(_ui->action_CloneSelected);
                menu.addAction(_ui->action_RemoveSelected);
            }
        }
        else {
            menu.addAction(_ui->action_AddFrame);
            menu.addAction(_ui->action_AddAnimation);
        }

        QPoint globalPos = _ui->treeView->mapToGlobal(pos);
        menu.exec(globalPos);
    }
}

void ExportOrderEditorWidget::showEditorForCurrentIndex()
{
    QModelIndex index = _ui->treeView->currentIndex();
    if (index.isValid()) {
        _ui->treeView->edit(index);
    }
}

void ExportOrderEditorWidget::closeEditor()
{
    QModelIndex index = _ui->treeView->currentIndex();
    if (index.isValid()) {
        _ui->treeView->closePersistentEditor(index);
    }
}

void ExportOrderEditorWidget::onActionAddFrame()
{
    Q_ASSERT(_exportOrder);
    _exportOrder->exportNameList()->editList_addFrame();
    showEditorForCurrentIndex();
}

void ExportOrderEditorWidget::onActionAddAnimation()
{
    Q_ASSERT(_exportOrder);
    _exportOrder->exportNameList()->editList_addAnimation();
    showEditorForCurrentIndex();
}

void ExportOrderEditorWidget::onActionAddAlternative()
{
    Q_ASSERT(_exportOrder);

    bool s = _exportOrder->alternativesList()->editSelectedList_addItem();
    if (s) {
        showEditorForCurrentIndex();
    }
}

void ExportOrderEditorWidget::onActionCloneSelected()
{
    using InternalIdFormat = ExportOrderModel::InternalIdFormat;

    Q_ASSERT(_exportOrder);

    QModelIndex index = _ui->treeView->currentIndex();
    if (index.isValid()) {
        InternalIdFormat id = index.internalId();

        if (id.index != InternalIdFormat::NO_INDEX) {
            bool s = false;
            if (id.altIndex == InternalIdFormat::NO_INDEX) {
                s = _exportOrder->exportNameList()->editSelectedList_cloneSelected();
            }
            else {
                s = _exportOrder->alternativesList()->editSelectedList_cloneSelected();
            }

            if (s) {
                showEditorForCurrentIndex();
            }
        }
    }
}

#define SELECTION_ACTION(ACTION_METHOD, ACCESSOR_METHOD)             \
    void ExportOrderEditorWidget::ACTION_METHOD()                    \
    {                                                                \
        using InternalIdFormat = ExportOrderModel::InternalIdFormat; \
                                                                     \
        Q_ASSERT(_exportOrder);                                      \
                                                                     \
        QModelIndex index = _ui->treeView->currentIndex();           \
        if (index.isValid()) {                                       \
            InternalIdFormat id = index.internalId();                \
            if (id.altIndex == InternalIdFormat::NO_INDEX) {         \
                _exportOrder->exportNameList()->ACCESSOR_METHOD();   \
            }                                                        \
            else {                                                   \
                _exportOrder->alternativesList()->ACCESSOR_METHOD(); \
            }                                                        \
        }                                                            \
    }

SELECTION_ACTION(onActionRemoveSelected, editSelectedList_removeSelected);
SELECTION_ACTION(onActionRaiseToTop, editSelectedList_raiseSelectedToTop);
SELECTION_ACTION(onActionRaise, editSelectedList_raiseSelected);
SELECTION_ACTION(onActionLower, editSelectedList_lowerSelected);
SELECTION_ACTION(onActionLowerToBottom, editSelectedList_lowerSelectedToBottom);
