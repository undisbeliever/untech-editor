/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#if defined(__clang__)
#define IGNORE_UNSIGNED_OVERFLOW_ATTR (no_sanitize("unsigned-integer-overflow"))
#else
#define IGNORE_UNSIGNED_OVERFLOW_ATTR ()
#endif
