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
class Document;
class AbstractResourceItem;

/**
 * The ResourceValidationWorker will call `AbstractResourceItem::validateItem()`
 * for all dirty items in the background so the user can see if the resource is
 * valid or contains errors.
 *
 * This is done with the QTimer with a 0ms interval so it will not block the
 * main thread.
 */
class ResourceValidationWorker : public QObject {
    Q_OBJECT

public:
    ResourceValidationWorker(Document* parent);

public slots:
    void validateAllResources();

private slots:
    void processNextResource();

private:
    Document* const _document;
    QList<AbstractResourceItem*> _itemsToProcess;
    QTimer _timer;
};
}
}
}
