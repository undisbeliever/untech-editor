/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"

namespace UnTech::Project {

enum MappingMode {
    LOROM,
    HIROM,
};

struct MemoryMapSettings {
    MappingMode mode = MappingMode::HIROM;
    unsigned firstBank = 0xc2;
    unsigned nBanks = 6;

    unsigned bankSize() const { return mode == MappingMode::HIROM ? 64 * 1024 : 32 * 1024; }

    unsigned bankAddress(const unsigned bankNumber) const
    {
        const unsigned a = (firstBank + bankNumber) * 0x10000;
        return MappingMode::HIROM ? a : a + 0x8000;
    }

    bool operator==(const MemoryMapSettings& o) const
    {
        return mode == o.mode
               && firstBank == o.firstBank
               && nBanks == o.nBanks;
    }
    bool operator!=(const MemoryMapSettings& o) const { return !(*this == o); }
};

}
