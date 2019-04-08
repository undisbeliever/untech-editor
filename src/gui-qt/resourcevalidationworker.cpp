/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcevalidationworker.h"
#include "abstractresourceitem.h"
#include "abstractresourcelist.h"
#include "project.h"

using namespace UnTech::GuiQt;

ResourceValidationWorker::ResourceValidationWorker(Project* project)
    : QObject(project)
    , _project(project)
{
    Q_ASSERT(project);

    _timer.setInterval(0);
    _timer.setSingleShot(true);

    connect(&_timer, &QTimer::timeout,
            this, &ResourceValidationWorker::processNextResource);

    connect(_project, &Project::resourceItemCreated,
            this, &ResourceValidationWorker::onResourceItemCreated);
    connect(_project, &Project::resourceItemAboutToBeRemoved,
            this, &ResourceValidationWorker::onResourceItemAboutToBeRemoved);
}

void ResourceValidationWorker::onResourceItemCreated(AbstractResourceItem* item)
{
    checkResourceLater(item);

    connect(item, &AbstractResourceItem::resourceLoaded,
            this, &ResourceValidationWorker::onResourceItemDependantsNeedChecking);
    connect(item, &AbstractResourceItem::resourceComplied,
            this, &ResourceValidationWorker::onResourceItemDependantsNeedChecking);
    connect(item, &AbstractResourceItem::nameAboutToChange,
            this, &ResourceValidationWorker::onResourceItemDependantsNeedChecking);

    connect(item, &AbstractResourceItem::stateChanged,
            this, &ResourceValidationWorker::onResourceItemStateChanged);
}

void ResourceValidationWorker::onResourceItemAboutToBeRemoved(AbstractResourceItem* item)
{
    item->disconnect(this);

    markItemDependantsUnchecked(item);

    _itemsToProcess.removeAll(item);
}

void ResourceValidationWorker::onResourceItemDependantsNeedChecking()
{
    if (auto* item = qobject_cast<AbstractResourceItem*>(sender())) {
        markItemDependantsUnchecked(item);
    }
}

void ResourceValidationWorker::onResourceItemStateChanged()
{
    if (auto* item = qobject_cast<AbstractResourceItem*>(sender())) {
        switch (item->state()) {
        case ResourceState::UNCHECKED:
        case ResourceState::NOT_LOADED:
            checkResourceLater(item);
            break;

        case ResourceState::ERROR:
        case ResourceState::FILE_ERROR:
        case ResourceState::DEPENDENCY_ERROR:
            markItemDependantsUnchecked(item);
            break;

        case ResourceState::VALID:
            break;
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

    // Process unloaded resources first
    for (AbstractResourceList* rl : _project->resourceLists()) {
        for (AbstractResourceItem* item : rl->items()) {
            if (item->state() == ResourceState::NOT_LOADED) {
                items.append(item);
            }
        }
    }

    for (AbstractResourceList* rl : _project->resourceLists()) {
        for (AbstractResourceItem* item : rl->items()) {
            if (item->state() != ResourceState::NOT_LOADED) {
                item->markUnchecked();
                items.append(item);
            }
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
        markItemDependantsUnchecked(toProcess);
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
    }

    if (!_itemsToProcess.isEmpty()) {
        _timer.start();
    }
}

void ResourceValidationWorker::markItemDependantsUnchecked(AbstractResourceItem* item)
{
    const AbstractResourceItem::Dependency toMatch = { item->resourceTypeIndex(), item->name() };

    for (AbstractResourceList* rl : _project->resourceLists()) {
        for (AbstractResourceItem* toTest : rl->items()) {
            if (toTest->dependencies().contains(toMatch)) {
                toTest->markUnchecked();
            }
        }
    }
}
