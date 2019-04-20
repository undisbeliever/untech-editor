/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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
class PropertyListModel;

class PropertyListManager : public QObject {
    Q_OBJECT

public:
    explicit PropertyListManager(QObject* parent = nullptr);
    ~PropertyListManager() = default;

    const QVector<Property>& propertiesList() const { return _properties; }

    const QString& propertyTitle(int id);

    void setEnabled(bool enabled);
    bool isEnabled() const { return _enabled; }

    /*
     * Called by `PropertyDelegate::setEditorData`.
     *
     * INPUT: id - id of the parameter being edited
     *        param1/param2 - the property's parameters
     *                        initially set to the property's default values
     */
    virtual void updateParameters(int id, QVariant& param1, QVariant& param2) const;

    virtual QVariant data(int id) const = 0;
    virtual bool setData(int id, const QVariant& value) = 0;

protected:
    // id must be >= 0
    void addProperty(const QString& title, int id, PropertyType type,
                     const QVariant& param1 = QVariant(), const QVariant& param2 = QVariant());

    void addPropertyGroup(const QString& title);

    // Replaces the propertyList
    void setPropertyList(const QVector<Property> properties);

signals:
    void propertyListChanged();
    void enabledChanged();

    // subclasses MUST emit this signal if the data changes
    void dataChanged();

    // subclesses should emit this signal if underlying list is about to change.
    // When emitted it will cause the PropertyDelegate to close all open editors.
    void listAboutToChange();

private:
    QVector<Property> _properties;
    bool _enabled;
};
}
}
