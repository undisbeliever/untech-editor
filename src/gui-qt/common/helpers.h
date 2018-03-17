/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/enummap.h"
#include "models/common/idstring.h"
#include <QStringList>
#include <QVariant>
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

inline std::vector<std::string> toStringVector(const QStringList& qsl)
{
    std::vector<std::string> sl;
    sl.reserve(qsl.size());
    std::transform(qsl.begin(), qsl.end(), std::back_inserter(sl),
                   [](const QString& qs) { return qs.toStdString(); });
    return sl;
}

inline std::vector<idstring> toIdstringVector(const QStringList& qsl)
{
    std::vector<idstring> sl;
    sl.reserve(qsl.size());
    std::transform(qsl.begin(), qsl.end(), std::back_inserter(sl),
                   [](const QString& qs) { return qs.toStdString(); });
    return sl;
}

// output is in the same order as `enumComboDataList`
template <typename EnumT>
QStringList enumComboNames(const EnumMap<EnumT>& enumMap)
{
    QStringList sl;
    sl.reserve(enumMap.size());
    std::transform(enumMap.begin(), enumMap.end(), std::back_inserter(sl),
                   [](const auto& p) { return QString::fromStdString(p.first); });
    return sl;
}

// output is in the same order as `enumComboNames`
template <typename EnumT>
QVariantList enumComboDataList(const EnumMap<EnumT>& enumMap)
{
    QVariantList vl;
    vl.reserve(enumMap.size());
    std::transform(enumMap.begin(), enumMap.end(), std::back_inserter(vl),
                   [](const auto& p) { return int(p.second); });
    return vl;
}
}
}
