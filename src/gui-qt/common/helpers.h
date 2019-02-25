/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/aabb.h"
#include "models/common/enummap.h"
#include "models/common/idstring.h"
#include "models/common/ms8aabb.h"
#include "models/common/namedlist.h"
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QStringList>
#include <QVariant>
#include <string>
#include <vector>

namespace UnTech {
namespace GuiQt {

inline QVariantList qVariantRange(int n)
{
    if (n < 0) {
        n = 0;
    }

    QVariantList list;
    list.reserve(n);

    for (int i = 0; i < n; i++) {
        list.append(i);
    }

    return list;
}

template <class T>
QStringList convertStringList(const T& sl)
{
    QStringList qsl;
    qsl.reserve(sl.size());
    std::transform(sl.begin(), sl.end(), std::back_inserter(qsl),
                   [](const std::string& s) { return QString::fromStdString(s); });
    return qsl;
}

template <typename T>
QStringList convertNameList(const NamedList<T>& nl)
{
    QStringList qsl;
    qsl.reserve(nl.size());
    std::transform(nl.begin(), nl.end(), std::back_inserter(qsl),
                   [](const T& i) { return QString::fromStdString(i.name); });
    return qsl;
}

template <typename T>
QStringList convertNameListWithBlank(const NamedList<T>& nl)
{
    QStringList qsl;
    qsl.reserve(nl.size() + 1);
    qsl.append(QString());
    std::transform(nl.begin(), nl.end(), std::back_inserter(qsl),
                   [](const T& i) { return QString::fromStdString(i.name); });
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

inline QPoint fromUpoint(const upoint& p)
{
    return QPoint(p.x, p.y);
}

inline upoint toUpoint(const QPoint& p)
{
    return upoint(p.x(), p.y());
}

inline QPoint fromMs8point(const ms8point& p)
{
    return QPoint(p.x, p.y);
}

inline ms8point toMs8point(const QPoint& p)
{
    return ms8point(p.x(), p.y());
}

inline QRect fromUrect(const urect& r)
{
    return QRect(r.x, r.y, r.width, r.height);
}

inline urect toUrect(const QRect& r)
{
    return urect(r.x(), r.y(), r.width(), r.height());
}

inline QRect fromMs8rect(const ms8rect& r)
{
    return QRect(r.x, r.y, r.width, r.height);
}

inline ms8rect toMs8rect(const QRect& r)
{
    return ms8rect(r.x(), r.y(), r.width(), r.height());
}

inline QSize fromUsize(const usize& s)
{
    return QSize(s.width, s.height);
}

inline usize toUsize(const QSize& s)
{
    return usize(s.width(), s.height());
}
}
}
