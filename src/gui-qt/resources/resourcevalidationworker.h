/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QList>
#include <QObject>
#include <QTimer>

namespace UnTech {
namespace GuiQt {
namespace Resources {
class ResourceProject;
class AbstractResourceItem;

/**
 * The ResourceValidationWorker will call `AbstractResourceItem::validateItem()`
 * for all unchecked items in the background so the user can see if the
 * resource is valid or contains errors.
 *
 * This is done with the QTimer with a 0ms interval so it will not block the
 * main thread.
 */
class ResourceValidationWorker : public QObject {
    Q_OBJECT

public:
    ResourceValidationWorker(ResourceProject* project);

    void checkResourceLater(AbstractResourceItem* item);

public slots:
    void validateAllResources();

private slots:
    void onResourceItemCreated(AbstractResourceItem* item);
    void onResourceItemAboutToBeRemoved(AbstractResourceItem* item);
    void onResourceItemStateChanged();

    void processNextResource();

private:
    ResourceProject* const _project;
    QList<AbstractResourceItem*> _itemsToProcess;
    QTimer _timer;
};
}
}
}
