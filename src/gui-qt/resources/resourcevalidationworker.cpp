/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcevalidationworker.h"
#include "abstractresourceitem.h"
#include "abstractresourcelist.h"
#include "document.h"

using namespace UnTech::GuiQt::Resources;

ResourceValidationWorker::ResourceValidationWorker(Document* document)
    : QObject(document)
    , _document(document)
{
    Q_ASSERT(document);

    _timer.setInterval(0);
    _timer.setSingleShot(true);

    connect(&_timer, &QTimer::timeout,
            this, &ResourceValidationWorker::processNextResource);
}

void ResourceValidationWorker::validateAllResources()
{
    _timer.stop();
    _itemsToProcess.clear();

    for (AbstractResourceList* rl : _document->resourceLists()) {
        for (AbstractResourceItem* item : rl->items()) {
            item->markUnchecked();
            _itemsToProcess.append(item);
        }
    }

    std::reverse(_itemsToProcess.begin(), _itemsToProcess.end());

    if (!_itemsToProcess.isEmpty()) {
        _timer.start();
    }
}

void ResourceValidationWorker::processNextResource()
{
    Q_ASSERT(_itemsToProcess.size() > 0);

    AbstractResourceItem* item = _itemsToProcess.takeLast();

    if (item->state() == ResourceState::NOT_LOADED) {
        item->loadResource();
    }
    item->validateItem();

    if (!_itemsToProcess.isEmpty()) {
        _timer.start();
    }
}
