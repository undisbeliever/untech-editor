/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <functional>
#include <tuple>

namespace UnTech {

template <typename Function, typename... Tuple, size_t... I>
auto call_dispatch(Function f, const std::tuple<Tuple...>& t, std::index_sequence<I...>)
{
    return f(std::get<I>(t)...);
}

template <typename Function, typename... Tuple>
auto call(Function f, const std::tuple<Tuple...>& t)
{
    return call_dispatch(f, t, std::index_sequence_for<Tuple...>{});
}

template <typename Function, class Class, typename... Tuple, size_t... I>
auto mem_fn_call_dispatch(Function f, Class* c, const std::tuple<Tuple...>& t, std::index_sequence<I...>)
{
    return f(c, std::get<I>(t)...);
}

template <typename Function, class Class, typename... Tuple>
auto mem_fn_call(Function f, Class* c, const std::tuple<Tuple...>& t)
{
    return mem_fn_call_dispatch(f, c, t, std::index_sequence_for<Tuple...>{});
}

template <typename Function, class Class, typename... Tuple, size_t... I, typename... Extras>
auto mem_fn_call_dispatch(Function f, Class* c, const std::tuple<Tuple...>& t, std::index_sequence<I...>, Extras... extra)
{
    return f(c, std::get<I>(t)..., extra...);
}

template <typename Function, class Class, typename... Tuple, typename... Extras>
auto mem_fn_call(Function f, Class* c, const std::tuple<Tuple...>& t, Extras... extra)
{
    return mem_fn_call_dispatch(f, c, t, std::index_sequence_for<Tuple...>{}, extra...);
}
}
