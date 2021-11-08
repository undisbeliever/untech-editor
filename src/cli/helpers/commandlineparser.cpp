/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "commandlineparser.h"
#include "version.h"
#include "models/common/iterators.h"
#include "models/common/string.h"
#include <cstdlib>
#include <iostream>

namespace UnTech::CommandLine {

OptionValue::OptionValue(bool boolean)
    : _type(Type::BOOLEAN)
    , _boolean(boolean)
    , _uint(boolean)
    , _path()
{
}

OptionValue::OptionValue(unsigned uint)
    : _type(Type::UNSIGNED)
    , _boolean(true)
    , _uint(uint)
    , _path()
{
}

OptionValue::OptionValue(std::filesystem::path&& p)
    : _type(Type::FILENAME)
    , _boolean(true)
    , _uint()
    , _path(std::move(p))
{
}

bool Argument::hasParameter() const
{
    switch (type) {
    case OptionType::FILENAME:
    case OptionType::UNSIGNED:
        return true;

    case OptionType::BOOLEAN:
    case OptionType::VERSION:
    case OptionType::HELP:
        return false;
    }

    abort();
}

const char* optionString(const OptionType type)
{
    switch (type) {
    case OptionType::BOOLEAN:
    case OptionType::VERSION:
    case OptionType::HELP:
        return "boolean";

    case OptionType::FILENAME:
        return "filename";

    case OptionType::UNSIGNED:
        return "number";
    }

    return "";
}

// PARSER
// ======

Parser::Parser(const Config& config)
    : _config(config)
    , _programExec()
    , _inputFilename()
    , _options()
{
}

void Parser::parse(int argc, const char* argv[])
{
    _programExec = argv[0];

    for (const auto& a : _config.arguments) {
        _options[a.longName] = a.defaultValue;
    }

    bool inSwitches = true;

    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];

        if (inSwitches && arg[0] == '-') {
            bool useNextArg = false;
            const char* nextArg = i + 1 < argc ? argv[i + 1] : nullptr;

            if (arg[1] == '-') {
                // long switch
                if (arg[2] == 0) {
                    inSwitches = false;
                    continue;
                }
                else {
                    useNextArg = parseLongSwitch(arg + 2, nextArg);
                }
            }
            else {
                useNextArg = parseShortSwitches(arg + 1, nextArg);
            }

            if (useNextArg) {
                i++;
            }
        }
        else {
            if (!_inputFilename.empty()) {
                error("Too many ", _config.inputFileType, " file arguments");
            }
            _inputFilename = arg;
        }
    }

    // verify required switches
    for (const auto& a : _config.arguments) {
        if (a.required && _options[a.longName] == false) {
            if (a.shortName) {
                error("Missing ", a.helpText, " (-", a.shortName, " / --", a.longName, ")");
            }
            else {
                error("Missing ", a.helpText, " (--", a.longName, ")");
            }
        }
    }

    if (_inputFilename.empty()) {
        error("Expected ", _config.inputFileType);
    }
}

bool Parser::parseShortSwitches(const char* currentArg, const char* nextArg)
{
    const char* arg = currentArg;

    if (*arg == '\0') {
        error("Expected argument after -");
    }

    while (*arg != '\0') {
        bool isValid = false;

        for (const auto& a : _config.arguments) {
            if (*arg == a.shortName) {
                if (arg[1] == 0) {
                    return parseSwitch(a, true, nextArg);
                }
                if (arg[1] == '=' && arg[2] != 0) {
                    parseSwitch(a, true, arg + 2);
                    return false;
                }
                else {
                    parseSwitch(a, true, nullptr);
                    isValid = true;
                    break;
                }
            }
        }

        if (isValid == false) {
            error("Unknown argument -", *arg);
        }
        arg++;
    }

    return false;
}

bool Parser::parseLongSwitch(const char* arg, const char* nextArg)
{
    for (const auto& a : _config.arguments) {
        if (arg == a.longName) {
            return parseSwitch(a, false, nextArg);
        }
    }

    error("Unknown argument --", arg);
    return false;
}

bool Parser::parseSwitch(const Argument& argument, bool isShort, const char* nextArg)
{
    if (argument.hasParameter()) {
        if (nextArg == nullptr) {
            error("Expected parameter", argument, isShort);
        }
        else if (nextArg[0] == '-') {
            error("Expected parameter", argument, isShort);
        }
    }

    switch (argument.type) {
    case OptionType::BOOLEAN:
        _options[argument.longName] = true;
        return false;

    case OptionType::FILENAME:
        _options[argument.longName] = std::filesystem::path(nextArg);
        return true;

    case OptionType::UNSIGNED: {
        const auto n = String::toUint32(nextArg);
        if (n) {
            _options[argument.longName] = n.value();
        }
        else {
            error("Require unsigned integer", argument, isShort);
        }
        return true;
    }

    case OptionType::VERSION:
        printVersion();
        exit(EXIT_SUCCESS);

    case OptionType::HELP:
        printHelpText();
        exit(EXIT_SUCCESS);
    }

    return false;
}

void Parser::printHelpText()
{
    auto printArgument = [](const auto& a) {
        int spacing = 24;

        std::cout << "\n  ";

        if (a.shortName) {
            std::cout << '-' << a.shortName << ' ';
            spacing -= 3;
        }
        std::cout << "--" << a.longName;
        spacing -= a.longName.size();

        if (a.type == OptionType::FILENAME) {
            std::cout << " <file>";
            spacing -= 7;
        }
        else if (a.type == OptionType::UNSIGNED) {
            std::cout << " <uint>";
            spacing -= 7;
        }

        if (!a.helpText.empty()) {
            for ([[maybe_unused]] const auto i : irange(spacing)) {
                std::cout << ' ';
            }
            std::cout << a.helpText;
        }

        if (a.defaultValue) {
            std::cout << " (default " << a.defaultValue << ")";
        }
    };

    std::cout << _config.programName << "\n\n";

    std::cout << "Usage: " << _programExec << " [options] <" << _config.inputFileType << ">\n";

    std::cout << "\nRequired arguments:";
    for (const auto& a : _config.arguments) {
        if (a.required) {
            printArgument(a);
        }
    }

    std::cout << "\n\nOptional arguments:";
    for (const auto& a : _config.arguments) {
        if (!a.required) {
            printArgument(a);
        }
    }

    std::cout << std::endl;
}

void Parser::printVersion()
{
    std::cout << _config.programName
              << " version " UNTECH_VERSION
                 "\n"
                 "\nPart of the " UNTECH_NAME " Editor Suite"
                 "\nLicensed under " UNTECH_LICENSE
                 "\n"
                 "\nThe " UNTECH_NAME " Editor Suite makes uses of the following third party source code:"
                 "\n\tLodePNG - Copyright (c) 2005-2020 Lode Vandevenne, zlib License"
                 "\n\tLZ4 Library - Copyright (c) 2011-2020, Yann Collet, BSD 2-Clause license"
                 "\n";
}

template <typename... Args>
inline void Parser::error(const Args... message)
{
    std::cerr << _programExec << ": ";

    (std::cerr << ... << message);

    std::cerr << std::endl;
    exit(EXIT_FAILURE);
}

void Parser::error(const char* message, const Argument& argument, bool isShort)
{
    std::cerr << _programExec << ": ";

    if (isShort) {
        std::cerr << "-" << argument.shortName << ": " << message << std::endl;
    }
    else {
        std::cerr << "--" << argument.longName << ": " << message << std::endl;
    }
    exit(EXIT_FAILURE);
}

}
