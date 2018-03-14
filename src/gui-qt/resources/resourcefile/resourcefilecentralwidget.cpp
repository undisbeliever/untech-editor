/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcefilecentralwidget.h"
#include "gui-qt/resources/document.h"
#include "gui-qt/resources/resourcefile/resourcefilecentralwidget.ui.h"

using namespace UnTech::GuiQt::Resources;

// ::TODO add some stats and total size figures in this widget::

ResourceFileCentralWidget::ResourceFileCentralWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::ResourceFileCentralWidget>())
    , _document(nullptr)
{
    _ui->setupUi(this);
}

ResourceFileCentralWidget::~ResourceFileCentralWidget() = default;

void ResourceFileCentralWidget::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
    }
    _document = document;
}
