/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "propertymanager.h"

#include <QLabel>
#include <QStringList>
#include <QToolButton>
#include <QWidget>

namespace UnTech {
namespace GuiQt {
class ColorToolButton;

class ListItemWidget : public QWidget {
    Q_OBJECT

public:
    ListItemWidget(const PropertyManager* manager, int propertyIndex,
                   QWidget* parent = nullptr);
    ~ListItemWidget() = default;

    const QStringList& stringList() const { return _stringList; }
    void setStringList(const QStringList& list);

private:
    void updateGui();

public slots:
    void onAddButtonClicked();

signals:
    void listEdited();

private:
    const PropertyManager* const _manager;
    const int _propertyIndex;

    QStringList _stringList;

    QLabel* _label;
    QToolButton* _addButton;
};
}
}
