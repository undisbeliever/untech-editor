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
    enum class Type {
        BOOLEAN,       // no parameters
        INTEGER,       // minimum value, maximum value
        UNSIGNED,      // minimum value, maximum value
        STRING,        // no parameters
        IDSTRING,      // completer stringlist
        FILENAME,      // dialog filter
        COLOR,         // no parameters
        COMBO,         // StringList of values, (optional) data VariantList
        STRING_LIST,   // no parameters
        IDSTRING_LIST, // completer values
        FILENAME_LIST  // dialog filter
    };

    struct Property {
        QString title;
        int id;
        Type type;

        QVariant parameter1;
        QVariant parameter2;

        bool isList;
    };
    const static Property blankProperty;

public:
    explicit PropertyManager(QObject* parent = nullptr);
    ~PropertyManager() = default;

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
    void addProperty(const QString& title, int id, Type type,
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
