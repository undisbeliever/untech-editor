/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources.h"
#include "version.h"
#include <cassert>
#include <functional>
#include <sstream>
#include <unordered_set>

namespace UnTech {
namespace Resources {

class ResourceWriter {
private:
    std::stringstream _incData;
    std::vector<uint8_t> _binaryData;
    bool _finalized;

public:
    ResourceWriter(const std::string& binaryFilename)
        : _incData()
        , _binaryData()
        , _finalized(false)
    {
        _incData << "namespace Resources {\n"
                 << "  insert __Data__, \"" << binaryFilename << "\"\n\n";
    }

    void writeConstant(const char* name, long value)
    {
        _incData << "constant " << name << " = " << value << '\n';
    }

    void startList(const char* exportListName)
    {
        _incData << '\n'
                 << exportListName << ":\n";
    }

    void addDataToList(const std::vector<uint8_t>& data)
    {
        _incData << "    dl __Data__ + " << _binaryData.size() << '\n';

        _binaryData.insert(_binaryData.end(), data.begin(), data.end());
    }

    template <class T>
    void writeNamesList(const std::vector<T>& inputList, const char* exportListName)
    {
        _incData << "\nnamespace " << exportListName << " {\n"
                 << "  constant count = " << inputList.size() << "\n\n";

        for (unsigned id = 0; id < inputList.size(); id++) {
            _incData << "  constant " << inputList.at(id).name << " = " << id << '\n';
        }

        _incData << "}\n";
    }

    void finalize()
    {
        assert(_finalized == false);
        _finalized = true;

        _incData << "\n}\n";
    }

    std::unique_ptr<ResourcesOutput> getOutput() const
    {
        assert(_finalized);

        auto ret = std::make_unique<ResourcesOutput>();

        ret->incData = _incData.str();
        ret->binaryData = _binaryData;

        return ret;
    }
};

std::unique_ptr<ResourcesOutput>
compileResources(const ResourcesFile& input, const std::string& binaryFilename)
{
    input.validate();

    ResourceWriter writer(binaryFilename);

    auto compileList = [&](const auto& inputList,
                           const char* exportListName, const char* typeName,
                           auto compile_fn) {

        writer.startList(exportListName);

        for (const auto& item : inputList) {
            try {
                std::vector<uint8_t> data = compile_fn(item);
                writer.addDataToList(data);
            }
            catch (const std::runtime_error& ex) {
                throw std::runtime_error(std::string("Unable to compile ")
                                         + typeName + ' ' + item.name + ": " + ex.what());
            }
        }
    };

    writer.writeConstant("UNTECH_VERSION", UNTECH_VERSION_INT);
    writer.writeConstant("PALETTE_FORMAT_VERSION", PaletteData::PALETTE_FORMAT_VERSION);

    compileList(input.palettes, "PaletteList", "palette",
                [](const PaletteInput& p) { return convertPalette(p).exportPalette(); });

    writer.writeNamesList(input.palettes, "PaletteList");

    writer.finalize();

    return writer.getOutput();
}
}
}
