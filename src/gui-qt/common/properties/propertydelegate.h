/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QStyledItemDelegate>

namespace UnTech {
namespace GuiQt {
class PropertyManager;

class PropertyDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    PropertyDelegate(QObject* parent = nullptr);
    ~PropertyDelegate() = default;

    virtual QString displayText(const QVariant& value, const QLocale& locale) const final;
};
}
}
