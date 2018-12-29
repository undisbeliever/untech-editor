/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "references.h"

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

// ::TODO generate debug file - containing frame/frameset names::

void writeFrameSetReferences(const Project& project, std::ostream& out)
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

void writeExportOrderReferences(const Project& project, std::ostream& out)
{
    out << "namespace MSEO {\n";

    for (const auto& it : project.exportOrders) {
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

}
}
}
