/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "property.h"
#include <QObject>
#include <QTreeView>
#include <QVariant>
#include <QVector>

namespace UnTech {
namespace GuiQt {
class PropertyTableModel;

class PropertyTableManager : public QObject {
    Q_OBJECT

public:
    explicit PropertyTableManager(QObject* parent = nullptr);
    ~PropertyTableManager() = default;

    const QVector<Property>& propertiesList() const { return _properties; }

    QStringList propertyTitles() const;

    const QString& propertyTitle(int id);

    const QString& title() const { return _title; }
    void setTitle(const QString& title);

    bool isEnabled() const { return _enabled; }
    void setEnabled(bool enabled);

    bool canMoveItems() const { return _canMoveItems; }

    /*
     * Called by `PropertyTableModel::propertyParametersForIndex`.
     *
     * This function does not change the parameters by default
     *
     * INPUT: id - id of the parameter being edited
     *        param1/param2 - the property's parameters
     *                        initially set to the property's default values
     */
    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const;

    virtual int rowCount() const = 0;
    virtual QVariant data(int index, int id) const = 0;
    virtual bool setData(int index, int id, const QVariant& value) = 0;

    // by default returns false
    virtual bool canInsertItem();
    virtual bool canCloneItem(int index);

    virtual bool insertItem(int index);
    virtual bool cloneItem(int index);
    virtual bool removeItem(int index);
    virtual bool moveItem(int from, int to);

protected:
    // Items are NOT movable by default
    // Set this true (and implement moveItem) if the manager supports moving items.
    void setItemsMovable(bool canMove);

    // if id is < 0 then the property cannot be edited
    void addProperty(const QString& title, int id, PropertyType type,
                     const QVariant& param1 = QVariant(), const QVariant& param2 = QVariant());

signals:
    void propertyListChanged();
    void titleChanged();
    void enabledChanged();
    void canMoveItemsChanged();

    void dataReset();

    // subclasses MUST emit this signal if the underlying data changes
    void dataChanged();

    // subclesses MUST emit this signal if underlying list is about to change.
    // When emitted it will cause the PropertyDelegate to close all open editors.
    void listAboutToChange();

    // subclasses MUST emit these signals if the table items change
    void itemChanged(int index);
    void itemAdded(int index);
    void itemRemoved(int index);
    void itemMoved(int from, int to);

private:
    QString _title;
    QVector<Property> _properties;
    bool _enabled;
    bool _canMoveItems;
};
}
}
