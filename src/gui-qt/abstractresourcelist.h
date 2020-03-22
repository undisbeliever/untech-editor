/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include <QObject>
#include <QVector>

namespace UnTech {
namespace GuiQt {
class Project;
class AbstractResourceItem;

class AbstractResourceList : public QObject {
    Q_OBJECT

public:
    struct AddResourceSettings {
        // If either filter or extension are empty then an inputbox containing
        // the name of the object will be shown instead of a file dialog.
        QString title;
        QString filter;
        QString extension;
        bool canCreateFile = true;
    };

public:
    AbstractResourceList(Project* project, ResourceTypeIndex typeIndex);
    ~AbstractResourceList() = default;

    ResourceTypeIndex resourceTypeIndex() const { return _resourceTypeIndex; }

    Project* project() const { return _project; }

    const QList<AbstractResourceItem*>& items() const { return _items; }
    const ResourceState& state() const { return _state; }

    QStringList itemNames() const;

    virtual const QString resourceTypeNameSingle() const = 0;
    virtual const QString resourceTypeNamePlural() const = 0;

    // MUST return the same list on every call
    virtual const QVector<AddResourceSettings>& addResourceSettings() const = 0;

    void addResource(int settingIndex, const QString& input);
    void removeResource(int index);

    // replaces the resource with new one and loads it to memory.
    // Returns the newly created item
    AbstractResourceItem* revertResource(AbstractResourceItem* item);

    AbstractResourceItem* findResource(const QString& name) const;

protected:
    // number of this type of data in the project.
    virtual size_t nItems() const = 0;

    virtual AbstractResourceItem* buildResourceItem(size_t index) = 0;

    // If the resource is external then create a new resource file.
    // Is allowed to throw an exception
    // settingIndex matches the index from addResourceSettings
    virtual void do_addResource(int settingIndex, const std::string& input) = 0;

    virtual void do_removeResource(unsigned index) = 0;

private:
    friend class Project;
    void rebuildResourceItems();

    // does not emit listChanged()
    void appendNewItemToList();
    void connectItemSignals(AbstractResourceItem* item);

private slots:
    void updateState();

signals:
    void listChanged();
    void stateChanged();
    void resourceItemCreated(AbstractResourceItem* item);
    void resourceItemAboutToBeRemoved(AbstractResourceItem* item);
    void resourceItemNameAboutToChange(AbstractResourceItem* item);

protected:
    Project* const _project;

private:
    const ResourceTypeIndex _resourceTypeIndex;

    ResourceState _state;
    QList<AbstractResourceItem*> _items;
};
}
}
