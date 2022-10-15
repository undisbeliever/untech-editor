/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "commandlineparser.h"
#include "version.h"
#include "models/common/iterators.h"
#include "models/common/string.h"
#include "models/common/u8strings.h"
#include <algorithm>
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

// PARSER
// ======

Parser::Parser(const Config& config)
    : _config(config)
    , _programExec()
    , _inputFilename()
    , _options()
{
}

void Parser::parse(int argc, const char** const argv)
{
    if (argc < 1 || argv == nullptr) {
        error("Invalid program arguments");
    }

    parse(std::span(argv, argc));
}

void Parser::parse(const std::span<const char*> arguments)
{
    using namespace std::literals;

    if (arguments.empty()) {
        error("Invalid program arguments");
    }
    for (auto a : arguments) {
        if (a == nullptr) {
            error("Invalid program argument");
        }
        if (std::string_view(a).empty()) {
            error("Invalid program argument");
        }
    }

    _programExec = arguments[0];

    for (const auto& a : _config.arguments) {
        _options[a.longName] = a.defaultValue;
    }

    bool inSwitches = true;

    for (size_t i = 1; i < arguments.size(); i++) {
        const std::string_view arg = arguments[i];

        if (inSwitches && arg.front() == '-') {
            const std::string_view nextArg = (i + 1 < arguments.size()) ? arguments[i + 1] : std::string_view();
            bool useNextArg = false;

            if (arg.starts_with("--"sv)) {
                if (arg.size() == 2) {
                    // arg == "--"
                    inSwitches = false;
                    continue;
                }
                else {
                    useNextArg = parseLongSwitch(arg.substr(2), nextArg);
                }
            }
            else {
                useNextArg = parseShortSwitches(arg.substr(1), nextArg);
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

bool Parser::parseShortSwitches(const std::string_view argument, const std::string_view nextArg)
{
    if (argument.empty()) {
        error("Expected argument after -");
    }

    auto arg = argument;

    while (!arg.empty()) {
        bool isValid = false;

        for (const auto& a : _config.arguments) {
            if (arg.front() == a.shortName) {
                if (arg.size() == 1) {
                    return parseSwitch(a, true, nextArg);
                }
                else if (arg[1] == '=') {
                    parseSwitch(a, true, arg.substr(2));
                    return false;
                }
                else {
                    parseSwitch(a, true, {});
                    isValid = true;
                    break;
                }
            }
        }

        if (isValid == false) {
            error("Unknown argument -", arg.front());
        }

        arg.remove_prefix(1);
    }

    return false;
}

bool Parser::parseLongSwitch(const std::string_view argument, const std::string_view nextArg)
{
    const auto& cArgs = _config.arguments;

    const auto it = std::find_if(cArgs.begin(), cArgs.end(),
                                 [&](const auto& a) { return argument == a.longName; });
    if (it != cArgs.end()) {
        return parseSwitch(*it, false, nextArg);
    }
    else {
        error("Unknown argument --", argument);
        return false;
    }
}

bool Parser::parseSwitch(const Argument& argument, bool isShort, const std::string_view nextArg)
{
    if (argument.hasParameter()) {
        if (nextArg.empty() || nextArg.front() == '-') {
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
        const auto n = String::toUint32(convert_old_string(nextArg));
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

void Parser::printHelpText() const
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

void Parser::printVersion() const
{
    std::cout << _config.programName
              << " version " UNTECH_VERSION
                 "\n"
                 "\nPart of the " UNTECH_NAME " Editor Suite"
                 "\nLicensed under " UNTECH_LICENSE
                 "\n"
                 "\nThe " UNTECH_NAME " Editor Suite makes uses of the following third party source code:"
                 "\n\tLodePNG - Copyright (c) 2005-2022 Lode Vandevenne, zlib License"
                 "\n\tLZ4 Library - Copyright (c) 2011-2020, Yann Collet, BSD 2-Clause license"
                 "\n";
}

template <typename... Args>
inline void Parser::error(const Args... message) const
{
    std::cerr << _programExec << ": ";

    (std::cerr << ... << message);

    std::cerr << std::endl;
    exit(EXIT_FAILURE);
}

void Parser::error(const char* message, const Argument& argument, bool isShort) const
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
