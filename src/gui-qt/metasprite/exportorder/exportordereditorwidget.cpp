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
        _exportOrder->disconnect(this);
    }
    _exportOrder = item;

    _model->setExportOrder(item);
    _ui->treeView->expandAll();

    this->setEnabled(item != nullptr);
}
