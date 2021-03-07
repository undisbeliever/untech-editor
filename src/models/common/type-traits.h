/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

template <class T>
struct remove_member_pointer {
};

template <typename T, class C>
struct remove_member_pointer<T C::*> {
    using type = T;
};

template <class T>
struct member_class {
};

template <typename T, class C>
struct member_class<T C::*> {
    using type = C;
};
