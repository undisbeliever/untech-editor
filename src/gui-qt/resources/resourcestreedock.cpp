/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcestreedock.h"
#include "document.h"
#include "resourcestreemodel.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/resources/resourcestreedock.ui.h"

using namespace UnTech::GuiQt::Resources;

ResourcesTreeDock::ResourcesTreeDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::ResourcesTreeDock)
    , _model(new ResourcesTreeModel)
    , _document(nullptr)
{
    _ui->setupUi(this);

    _ui->resourcesTree->setModel(_model);

    setEnabled(false);

    connect(_ui->resourcesTree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ResourcesTreeDock::onResourcesTreeSelectionChanged);
}

ResourcesTreeDock::~ResourcesTreeDock() = default;

void ResourcesTreeDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
    }
    _document = document;

    _model->setDocument(document);

    if (_document) {
        connect(_document, &Document::selectedResourceChanged,
                this, &ResourcesTreeDock::onSelectedResourceChanged);
    }

    setEnabled(_document != nullptr);
}

void ResourcesTreeDock::onSelectedResourceChanged()
{
    QModelIndex index = _model->toModelIndex(_document->selectedResource());
    _ui->resourcesTree->setCurrentIndex(index);
}

void ResourcesTreeDock::onResourcesTreeSelectionChanged()
{
    if (_document == nullptr) {
        return;
    }

    QModelIndex index = _ui->resourcesTree->currentIndex();
    AbstractResourceItem* item = _model->toResourceItem(index);
    _document->setSelectedResource(item);
}
