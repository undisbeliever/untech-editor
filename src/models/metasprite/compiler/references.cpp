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
    out.write("namespace MSFS {\n");

    auto writeRef = [&](const auto& fs, unsigned i) {
        out.write("\tconstant ", fs->name, " = ", i, "\n");
        out.write("\tdefine ", fs->name, ".type = ", fs->exportOrder, "\n");
    };

    for (auto [i, fs] : const_enumerate(project.frameSets)) {
        if (fs.siFrameSet) {
            writeRef(fs.siFrameSet, i);
        }
        else if (fs.msFrameSet) {
            writeRef(fs.msFrameSet, i);
        }
    }

    out.write("}\n");
}

void writeExportOrderReferences(const ProjectFile& project, StringStream& out)
{
    out.write("namespace MSEO {\n");

    for (const auto& it : project.frameSetExportOrders) {
        const auto& eo = it.value;
        if (eo == nullptr) {
            throw runtime_error("Unable to read Export Order: ", it.filename.string());
        }

        out.write("\tnamespace ", eo->name, " {\n");

        if (eo->stillFrames.size() > 0) {
            unsigned id = 0;
            out.write("\t\tnamespace Frames {\n");
            for (const auto& f : eo->stillFrames) {
                out.write("\t\t\tconstant ", f.name, " = ", id, "\n");
                id++;
            }
            out.write("\t\t}\n");
        }
        if (eo->animations.size() > 0) {
            unsigned id = 0;
            out.write("\t\tnamespace Animations {\n");
            for (const auto& a : eo->animations) {
                out.write("\t\t\tconstant ", a.name, " = ", id, "\n");
                id++;
            }
            out.write("\t\t}\n");
        }
        out.write("\t}\n");
    }

    out.write("}\n");
}

void writeActionPointFunctionTables(const NamedList<ActionPointFunction>& actionPointFunctions, StringStream& out)
{
    static constexpr unsigned WORD_SIZE = 2;

    assert(actionPointFunctions.size() <= MAX_ACTION_POINT_FUNCTIONS);

    out.write("\n"
              "code()\n"
              "Project.ActionPoints:\n"
              "namespace Project.ActionPoints {\n"
              "\tdw ActionPoints.InvalidActionPoint\n");

    bool hasManuallyInvokedFunction = false;
    for (const ActionPointFunction& ap : actionPointFunctions) {
        out.write("\tdw ActionPoints.", ap.name, "\n");

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
        out.write("\tdw ActionPoints.Null\n");
    }

    // constants/defines
    out.write("\n"
              "\tconstant MASK = ",
              (nextPowerOf2 * WORD_SIZE - 1 - 1), "\n");

    if (hasManuallyInvokedFunction) {
        out.write("\n");

        for (auto [i, ap] : const_enumerate(actionPointFunctions)) {
            if (ap.manuallyInvoked) {
                unsigned romValue = (i + 1) * 2;
                assert(romValue <= 255 - 2);

                out.write("\tdefine ", ap.name, " = ", romValue,
                          "\n\tconstant ", ap.name, " = ", romValue, "\n");
            }
        }
    }

    out.write("}\n");
}

}
