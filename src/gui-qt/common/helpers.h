/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QStringList>
#include <string>
#include <vector>

namespace UnTech {
namespace GuiQt {

template <class T>
QStringList convertStringList(const std::vector<T>& sl)
{
    QStringList qsl;
    qsl.reserve(sl.size());
    std::transform(sl.begin(), sl.end(), std::back_inserter(qsl),
                   [](const std::string& s) { return QString::fromStdString(s); });
    return qsl;
}
}
}
