/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include <QObject>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace Resources {
class Document;
class AbstractResourceItem;

class AbstractResourceList : public QObject {
    Q_OBJECT

public:
    AbstractResourceList(QObject* parent, ResourceTypeIndex typeIndex);
    ~AbstractResourceList() = default;

    ResourceTypeIndex resourceTypeIndex() const { return _resourceTypeIndex; }

    const QVector<AbstractResourceItem*>& list() const { return _list; }

    void setDocument(Document* document);
    Document* document() const { return _document; }

    virtual const QString resourceTypeName() const = 0;

protected:
    // number of this type of data in the document.
    virtual size_t nItems() const = 0;

    virtual AbstractResourceItem* buildResourceItem(size_t index) = 0;

protected:
    QVector<AbstractResourceItem*> _list;

private:
    Document* _document;
    const ResourceTypeIndex _resourceTypeIndex;
};
}
}
}
