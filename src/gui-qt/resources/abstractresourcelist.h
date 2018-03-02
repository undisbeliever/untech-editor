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
    struct AddResourceDialogSettings {
        // If either filter or extension are empty then an inputbox containing
        // the name of the object will be shown instead of a file dialog.
        QString title;
        QString filter;
        QString extension;
        bool canCreateFile;
    };

public:
    AbstractResourceList(QObject* parent, ResourceTypeIndex typeIndex);
    ~AbstractResourceList() = default;

    ResourceTypeIndex resourceTypeIndex() const { return _resourceTypeIndex; }

    void setDocument(Document* document);
    Document* document() const { return _document; }

    const QVector<AbstractResourceItem*>& items() const { return _items; }
    const ResourceState& state() const { return _state; }

    void addResource(const QString& input);
    void removeResource(int index);

    virtual const QString resourceTypeNameSingle() const = 0;
    virtual const QString resourceTypeNamePlural() const = 0;
    virtual const AddResourceDialogSettings& addResourceDialogSettings() const = 0;

protected:
    // number of this type of data in the document.
    virtual size_t nItems() const = 0;

    virtual AbstractResourceItem* buildResourceItem(size_t index) = 0;

    // If the resource is external then create a new resource file.
    // Is allowed to throw an exception
    virtual void do_addResource(const std::string& input) = 0;

    virtual void do_removeResource(unsigned index) = 0;

private:
    // does not emit listChanged()
    void appendNewItemToList(int index);

private slots:
    void updateState();

signals:
    void listChanged();
    void stateChanged();
    void resourceItemCreated(AbstractResourceItem* item);

private:
    Document* _document;
    const ResourceTypeIndex _resourceTypeIndex;
    ResourceState _state;
    QVector<AbstractResourceItem*> _items;
};
}
}
}
