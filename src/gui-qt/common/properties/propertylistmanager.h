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
    // if id is < 0 then the property is blank and cannot be selected
    void addProperty(const QString& title, int id, PropertyType type,
                     const QVariant& param1 = QVariant(), const QVariant& param2 = QVariant());

    void addSeperator(const QString& title);

signals:
    void propertyListChanged();
    void enabledChanged();

    // subclasses MUST emit this signal if the data changes
    void dataChanged();

private:
    QVector<Property> _properties;
    bool _enabled;
};
}
}
