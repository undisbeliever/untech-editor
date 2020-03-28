/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/enummap.h"
#include <QComboBox>
#include <map>
#include <string>

namespace UnTech {
namespace GuiQt {

class EnumComboBox : public QComboBox {
    Q_OBJECT

public:
    EnumComboBox(QWidget* parent = nullptr)
        : QComboBox(parent)
    {
    }
    ~EnumComboBox() = default;

public:
    template <typename EnumT>
    inline void populateData(const EnumMap<EnumT>& enumMap)
    {
        for (auto& it : enumMap) {
            QString s = QString::fromStdString(it.first);
            this->addItem(s, int(it.second));
        }
    }

    template <class EnumClassT>
    inline EnumClassT currentEnum()
    {
        return (typename EnumClassT::Enum)currentData().toInt();
    }

    inline int currentEnumInt()
    {
        return currentData().toInt();
    }

    template <class EnumClassT>
    inline void setCurrentEnum(const EnumClassT& e)
    {
        int index = this->findData(int(e.value()));
        this->setCurrentIndex(index);
    }

    inline void setCurrentEnum(const int v)
    {
        int index = this->findData(v);
        this->setCurrentIndex(index);
    }
};
}
}
