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

    setEnabled(_document != nullptr);
}
