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

    void setDocument(Document* document);
    Document* document() const { return _document; }

    const QVector<AbstractResourceItem*>& items() const { return _items; }
    const ResourceState& state() const { return _state; }

    virtual const QString resourceTypeNameSingle() const = 0;
    virtual const QString resourceTypeNamePlural() const = 0;

protected:
    // number of this type of data in the document.
    virtual size_t nItems() const = 0;

    virtual AbstractResourceItem* buildResourceItem(size_t index) = 0;

private slots:
    void updateState();

signals:
    void stateChanged();

private:
    Document* _document;
    const ResourceTypeIndex _resourceTypeIndex;
    ResourceState _state;
    QVector<AbstractResourceItem*> _items;
};
}
}
}
