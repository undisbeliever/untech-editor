/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "references.h"

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

using ProjectFile = UnTech::Project::ProjectFile;

// ::TODO generate debug file - containing frame/frameset names::

void writeFrameSetReferences(const ProjectFile& project, std::ostream& out)
{
    out << "namespace MSFS {\n";

    for (unsigned i = 0; i < project.frameSets.size(); i++) {
        auto writeRef = [&](const auto& fs) {
            out << "\tconstant " << fs->name << " = " << i << "\n";
            out << "\tdefine " << fs->name << ".type = " << fs->exportOrder << "\n";
        };

        const auto& fs = project.frameSets.at(i);
        if (fs.siFrameSet) {
            writeRef(fs.siFrameSet);
        }
        else if (fs.msFrameSet) {
            writeRef(fs.msFrameSet);
        }
    }

    out << "}\n";
}

void writeExportOrderReferences(const ProjectFile& project, std::ostream& out)
{
    out << "namespace MSEO {\n";

    for (const auto& it : project.frameSetExportOrders) {
        const FrameSetExportOrder* eo = it.value.get();
        if (eo == nullptr) {
            throw std::runtime_error("Unable to read Export Order: " + it.filename);
        }

        out << "\tnamespace " << eo->name << " {\n";

        if (eo->stillFrames.size() > 0) {
            unsigned id = 0;
            out << "\t\tnamespace Frames {\n";
            for (const auto& f : eo->stillFrames) {
                out << "\t\t\tconstant " << f.name << " = " << id << "\n";
                id++;
            }
            out << "\t\t}\n";
        }
        if (eo->animations.size() > 0) {
            unsigned id = 0;
            out << "\t\tnamespace Animations {\n";
            for (const auto& a : eo->animations) {
                out << "\t\t\tconstant " << a.name << " = " << id << "\n";
                id++;
            }
            out << "\t\t}\n";
        }
        out << "\t}\n";
    }

    out << "}\n";
}

void writeActionPointFunctionTables(const NamedList<ActionPointFunction>& actionPointFunctions, std::ostream& out)
{
    static constexpr unsigned WORD_SIZE = 2;

    assert(actionPointFunctions.size() <= MAX_ACTION_POINT_FUNCTIONS);

    out << "\n"
           "code()\n"
           "Project.ActionPoints:\n"
           "namespace Project.ActionPoints {\n"
           "\tdw ActionPoints.InvalidActionPoint\n";

    bool hasManuallyInvokedFunction = false;
    for (const ActionPointFunction& ap : actionPointFunctions) {
        out << "\tdw ActionPoints." << ap.name << '\n';

        hasManuallyInvokedFunction |= ap.manuallyInvoked;
    }

    // padding
    const unsigned nFunctions = actionPointFunctions.size() + 1;
    unsigned nextPowerOf2 = 1;
    while (nextPowerOf2 < nFunctions) {
        nextPowerOf2 <<= 1;
    }
    assert(nextPowerOf2 < 256 / WORD_SIZE);

    for (unsigned i = nFunctions; i < nextPowerOf2; i++) {
        out << "\tdw ActionPoints.Null\n";
    }

    // constants/defines
    out << "\n"
           "\tconstant MASK = "
        << (nextPowerOf2 * WORD_SIZE - 1 - 1) << '\n';

    if (hasManuallyInvokedFunction) {
        out << "\n";

        for (unsigned i = 0; i < actionPointFunctions.size(); i++) {
            const ActionPointFunction& ap = actionPointFunctions.at(i);

            if (ap.manuallyInvoked) {
                unsigned romValue = (i + 1) * 2;
                assert(romValue <= 255 - 2);

                out << "\tdefine " << ap.name << " = " << romValue << '\n'
                    << "\tconstant " << ap.name << " = " << romValue << '\n';
            }
        }
    }

    out << "}\n";
}

}
}
}
