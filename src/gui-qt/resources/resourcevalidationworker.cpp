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

    connect(_document, &Document::resourceItemCreated,
            this, &ResourceValidationWorker::onResourceItemCreated);
}

void ResourceValidationWorker::onResourceItemCreated(AbstractResourceItem* item)
{
    checkResourceLater(item);

    connect(item, &AbstractResourceItem::stateChanged,
            this, &ResourceValidationWorker::onResourceItemStateChanged);
    connect(item, &AbstractResourceItem::destroyed,
            this, &ResourceValidationWorker::onResourceItemDestroyed);
}

void ResourceValidationWorker::onResourceItemDestroyed(QObject* item)
{
    _itemsToProcess.removeAll(static_cast<AbstractExternalResourceItem*>(item));
}

void ResourceValidationWorker::onResourceItemStateChanged()
{
    auto* item = qobject_cast<AbstractResourceItem*>(sender());

    if (item) {
        if (item->state() == ResourceState::UNCHECKED
            || item->state() == ResourceState::NOT_LOADED) {

            checkResourceLater(item);
        }
    }
}

void ResourceValidationWorker::checkResourceLater(AbstractResourceItem* item)
{
    _itemsToProcess.removeAll(item);
    _itemsToProcess.append(item);

    if (!_timer.isActive()) {
        _timer.start();
    }
}

void ResourceValidationWorker::validateAllResources()
{
    _timer.stop();
    QList<AbstractResourceItem*> items;

    for (AbstractResourceList* rl : _document->resourceLists()) {
        for (AbstractResourceItem* item : rl->items()) {
            item->markUnchecked();
            items.append(item);
        }
    }

    std::reverse(items.begin(), items.end());

    if (!items.isEmpty()) {
        _itemsToProcess = items;
        _timer.start();
    }
}

void ResourceValidationWorker::processNextResource()
{
    if (_itemsToProcess.size() == 0) {
        _timer.stop();
        return;
    }

    AbstractResourceItem* item = _itemsToProcess.takeLast();

    if (item->state() == ResourceState::NOT_LOADED) {
        item->loadResource();
    }
    item->validateItem();

    if (!_itemsToProcess.isEmpty()) {
        _timer.start();
    }
}
