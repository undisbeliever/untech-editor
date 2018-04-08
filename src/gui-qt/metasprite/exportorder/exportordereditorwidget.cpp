/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "exportordereditorwidget.h"
#include "exportordermodel.h"
#include "exportorderresourceitem.h"
#include "gui-qt/common/properties/propertydelegate.h"
#include "gui-qt/metasprite/exportorder/exportordereditorwidget.ui.h"

#include "exportorderlists.h"
#include "gui-qt/undo/listactionhelper.h"
#include "gui-qt/undo/listundohelper.h"

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
    using namespace UnTech::GuiQt::Undo;

    Q_ASSERT(_exportOrder);

    auto* enList = _exportOrder->exportNameList();
    auto* altList = _exportOrder->alternativesList();

    ListActionStatus enStatus = ListActionHelper::status(enList);
    ListActionStatus altStatus = ListActionHelper::status(altList);

    ListActionStatus selStatus = altStatus.selectionValid ? altStatus : enStatus;

    _ui->action_AddFrame->setEnabled(ListActionHelper::canAddToList(enList, true));
    _ui->action_AddAnimation->setEnabled(ListActionHelper::canAddToList(enList, false));
    _ui->action_AddAlternative->setEnabled(altStatus.canAdd);
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

void ExportOrderEditorWidget::addExportName(bool isFrame)
{
    Q_ASSERT(_exportOrder);

    _exportOrder->exportNameList()->setSelectedListIsFrame(isFrame);
    ExportNameUndoHelper(_exportOrder->exportNameList()).addItemToSelectedList();

    showEditorForCurrentIndex();
}

void ExportOrderEditorWidget::onActionAddFrame()
{
    addExportName(true);
}

void ExportOrderEditorWidget::onActionAddAnimation()
{
    addExportName(false);
}

void ExportOrderEditorWidget::onActionAddAlternative()
{
    Q_ASSERT(_exportOrder);

    AlternativesUndoHelper(_exportOrder->alternativesList()).addItemToSelectedList();
    showEditorForCurrentIndex();
}

void ExportOrderEditorWidget::onActionCloneSelected()
{
    using InternalIdFormat = ExportOrderModel::InternalIdFormat;

    Q_ASSERT(_exportOrder);

    QModelIndex index = _ui->treeView->currentIndex();
    if (index.isValid()) {
        InternalIdFormat id = index.internalId();

        if (id.index != InternalIdFormat::NO_INDEX) {
            if (id.altIndex == InternalIdFormat::NO_INDEX) {
                ExportNameUndoHelper(_exportOrder->exportNameList()).cloneSelectedItem();
            }
            else {
                AlternativesUndoHelper(_exportOrder->alternativesList()).cloneSelectedItem();
            }

            showEditorForCurrentIndex();
        }
    }
}

void ExportOrderEditorWidget::onActionRemoveSelected()
{
    using InternalIdFormat = ExportOrderModel::InternalIdFormat;

    Q_ASSERT(_exportOrder);

    QModelIndex index = _ui->treeView->currentIndex();
    if (index.isValid()) {
        InternalIdFormat id = index.internalId();
        if (id.altIndex == InternalIdFormat::NO_INDEX) {
            ExportNameUndoHelper(_exportOrder->exportNameList()).removeSelectedItem();
        }
        else {
            AlternativesUndoHelper(_exportOrder->alternativesList()).removeSelectedItem();
        }
    }
}
