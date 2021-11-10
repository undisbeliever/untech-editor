/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "references.h"
#include "models/common/exceptions.h"
#include "models/common/iterators.h"
#include "models/common/stringstream.h"

namespace UnTech::MetaSprite::Compiler {

using ProjectFile = UnTech::Project::ProjectFile;

// ::TODO generate debug file - containing frame/frameset names::

void writeFrameSetReferences(const ProjectFile& project, StringStream& out)
{
    out.write(u8"namespace MSFS {\n");

    auto writeRef = [&](const auto& fs, unsigned i) {
        out.write(u8"\tconstant ", fs->name, u8" = ", i, u8"\n");
        out.write(u8"\tdefine ", fs->name, u8".type = ", fs->exportOrder, u8"\n");
    };

    for (auto [i, fs] : const_enumerate(project.frameSets)) {
        if (fs.siFrameSet) {
            writeRef(fs.siFrameSet, i);
        }
        else if (fs.msFrameSet) {
            writeRef(fs.msFrameSet, i);
        }
    }

    out.write(u8"}\n");
}

void writeExportOrderReferences(const ProjectFile& project, StringStream& out)
{
    out.write(u8"namespace MSEO {\n");

    for (const auto& it : project.frameSetExportOrders) {
        const auto& eo = it.value;
        if (eo == nullptr) {
            throw runtime_error(u8"Unable to read Export Order: ", it.filename.u8string());
        }

        out.write(u8"\tnamespace ", eo->name, u8" {\n");

        if (eo->stillFrames.size() > 0) {
            unsigned id = 0;
            out.write(u8"\t\tnamespace Frames {\n");
            for (const auto& f : eo->stillFrames) {
                out.write(u8"\t\t\tconstant ", f.name, u8" = ", id, u8"\n");
                id++;
            }
            out.write(u8"\t\t}\n");
        }
        if (eo->animations.size() > 0) {
            unsigned id = 0;
            out.write(u8"\t\tnamespace Animations {\n");
            for (const auto& a : eo->animations) {
                out.write(u8"\t\t\tconstant ", a.name, u8" = ", id, u8"\n");
                id++;
            }
            out.write(u8"\t\t}\n");
        }
        out.write(u8"\t}\n");
    }

    out.write(u8"}\n");
}

void writeActionPointFunctionTables(const NamedList<ActionPointFunction>& actionPointFunctions, StringStream& out)
{
    static constexpr unsigned WORD_SIZE = 2;

    assert(actionPointFunctions.size() <= MAX_ACTION_POINT_FUNCTIONS);

    out.write(u8"\n"
              u8"code()\n"
              u8"Project.ActionPoints:\n"
              u8"namespace Project.ActionPoints {\n"
              u8"\tdw ActionPoints.InvalidActionPoint\n");

    bool hasManuallyInvokedFunction = false;
    for (const ActionPointFunction& ap : actionPointFunctions) {
        out.write(u8"\tdw ActionPoints.", ap.name, u8"\n");

        hasManuallyInvokedFunction |= ap.manuallyInvoked;
    }

    // padding
    const unsigned nFunctions = actionPointFunctions.size() + 1;
    unsigned nextPowerOf2 = 1;
    while (nextPowerOf2 < nFunctions) {
        nextPowerOf2 <<= 1;
    }
    assert(nextPowerOf2 < 256 / WORD_SIZE);

    for ([[maybe_unused]] const auto i : range(nFunctions, nextPowerOf2)) {
        out.write(u8"\tdw ActionPoints.Null\n");
    }

    // constants/defines
    out.write(u8"\n"
              u8"\tconstant MASK = ",
              (nextPowerOf2 * WORD_SIZE - 1 - 1), u8"\n");

    if (hasManuallyInvokedFunction) {
        out.write(u8"\n");

        for (auto [i, ap] : const_enumerate(actionPointFunctions)) {
            if (ap.manuallyInvoked) {
                unsigned romValue = (i + 1) * 2;
                assert(romValue <= 255 - 2);

                out.write(u8"\tdefine ", ap.name, u8" = ", romValue,
                          u8"\n\tconstant ", ap.name, u8" = ", romValue, u8"\n");
            }
        }
    }

    out.write(u8"}\n");
}

}
