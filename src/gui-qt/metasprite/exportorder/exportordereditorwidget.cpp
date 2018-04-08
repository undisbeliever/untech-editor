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

using namespace UnTech::GuiQt::MetaSprite;

ExportOrderEditorWidget::ExportOrderEditorWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::ExportOrderEditorWidget>())
    , _model(new ExportOrderModel(this))
    , _exportOrder(nullptr)
{
    _ui->setupUi(this);

    _ui->treeView->setModel(_model);

    _ui->treeView->setItemDelegate(new PropertyDelegate(this));
    _ui->treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    _ui->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _ui->treeView->setAlternatingRowColors(true);
    _ui->treeView->header()->hide();
    _ui->treeView->header()->setStretchLastSection(false);
    _ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    _ui->treeView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    _ui->treeView->header()->resizeSection(1, 65);
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

    this->setEnabled(item != nullptr);

    if (_exportOrder) {
        updateSelection();

        connect(_exportOrder->exportNameList(), &ExportOrder::ExportNameList::selectedIndexChanged,
                this, &ExportOrderEditorWidget::updateSelection);
        connect(_exportOrder->alternativesList(), &ExportOrder::AlternativesList::selectedIndexChanged,
                this, &ExportOrderEditorWidget::updateSelection);

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
