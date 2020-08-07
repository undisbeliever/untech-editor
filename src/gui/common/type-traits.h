/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

template <class T>
struct remove_member_pointer {
};

template <typename T, class C>
struct remove_member_pointer<T C::*> {
    using type = T;
};
