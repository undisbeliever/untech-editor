/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcevalidationworker.h"
#include "abstractproject.h"
#include "abstractresourceitem.h"
#include "abstractresourcelist.h"

using namespace UnTech::GuiQt;

ResourceValidationWorker::ResourceValidationWorker(AbstractProject* project)
    : QObject(project)
    , _project(project)
{
    Q_ASSERT(project);

    _timer.setInterval(0);
    _timer.setSingleShot(true);

    connect(&_timer, &QTimer::timeout,
            this, &ResourceValidationWorker::processNextResource);

    connect(_project, &AbstractProject::resourceItemCreated,
            this, &ResourceValidationWorker::onResourceItemCreated);
    connect(_project, &AbstractProject::resourceItemAboutToBeRemoved,
            this, &ResourceValidationWorker::onResourceItemAboutToBeRemoved);

    connect(_project, &AbstractProject::resourceFileSettingsChanged,
            this, &ResourceValidationWorker::validateAllResources);
}

void ResourceValidationWorker::onResourceItemCreated(AbstractResourceItem* item)
{
    checkResourceLater(item);

    connect(item, &AbstractResourceItem::stateChanged,
            this, &ResourceValidationWorker::onResourceItemStateChanged);
}

void ResourceValidationWorker::onResourceItemAboutToBeRemoved(AbstractResourceItem* item)
{
    _itemsToProcess.removeAll(item);
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

    for (AbstractResourceList* rl : _project->resourceLists()) {
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

    AbstractResourceItem* toProcess = _itemsToProcess.last();

    if (toProcess->state() == ResourceState::NOT_LOADED) {
        toProcess->loadResource();
    }

    bool allDependenciesChecked = true;
    for (auto& dep : toProcess->dependencies()) {
        if (auto* d = _project->findResourceItem(dep.type, dep.name)) {
            if (d->state() == ResourceState::UNCHECKED
                || d->state() == ResourceState::NOT_LOADED) {

                _itemsToProcess.append(d);
                allDependenciesChecked = false;
            }
        }
    }

    if (allDependenciesChecked) {
        toProcess->validateItem();

        _itemsToProcess.removeAll(toProcess);

        if (!_itemsToProcess.isEmpty()) {
            _timer.start();
        }
    }
}
