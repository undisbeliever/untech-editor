/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QObject>
#include <QTreeView>
#include <QVariant>
#include <QVector>

namespace UnTech {
namespace GuiQt {
class PropertyModel;

class PropertyManager : public QObject {
    Q_OBJECT

public:
    struct Property {
        QString title;
        int id;
    };

public:
    explicit PropertyManager(QObject* parent = nullptr);
    ~PropertyManager() = default;

    const QVector<Property>& propertiesList() const { return _properties; }

    void setEnabled(bool enabled);
    bool isEnabled() const { return _enabled; }

    virtual QVariant data(int id) const = 0;
    virtual bool setData(int id, const QVariant& value) = 0;

protected:
    // if id is < 0 then the property is blank and cannot be selected
    void addProperty(const QString& title, int id);

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
