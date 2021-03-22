/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "xml.h"
#include <algorithm>

namespace UnTech::Xml {

std::string escape(const std::string_view text)
{
    using namespace std::string_view_literals;

    constexpr std::string_view toMatch = "&<>\"\'"sv;

    std::string ret;
    ret.reserve(text.size() + 64);

    auto start = text.begin();
    auto it = std::find_first_of(start, text.end(),
                                 toMatch.begin(), toMatch.end());

    while (it != text.end()) {
        ret.append(start, it);

        const char c = *it;
        switch (c) {
        case '&':
            ret.append("&amp;"sv);
            break;

        case '<':
            ret.append("&lt;"sv);
            break;

        case '>':
            ret.append("&gt;"sv);
            break;

        case '"':
            ret.append("&quot;"sv);
            break;

        case '\'':
            ret.append("&apos;"sv);
            break;

        default:
            abort();
        }

        start = it + 1;
        it = std::find_first_of(start, text.end(),
                                toMatch.begin(), toMatch.end());
    }

    ret.append(start, text.end());

    return ret;
}

}
