/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

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
    inline void populateData(const std::map<EnumT, std::string>& enumMap)
    {
        for (auto& it : enumMap) {
            QString s = QString::fromStdString(it.second);
            this->addItem(s, int(it.first));
        }
    }

    template <class EnumClassT>
    inline EnumClassT currentEnum()
    {
        return (typename EnumClassT::Enum)currentData().toInt();
    }

    template <class EnumClassT>
    inline void setCurrentEnum(const EnumClassT& e)
    {
        int index = this->findData(int(e.value()));
        this->setCurrentIndex(index);
    }
};
}
}
