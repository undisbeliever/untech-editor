#include "commandlineparser.h"
#include "version.h"
#include "models/common/string.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;
using namespace UnTech::CommandLine;

OptionValue::OptionValue(bool boolean)
    : _type(Type::BOOLEAN)
    , _boolean(boolean)
    , _uint(boolean)
    , _string()
{
}

OptionValue::OptionValue(unsigned uint)
    : _type(Type::UNSIGNED)
    , _boolean(true)
    , _uint(uint)
    , _string()
{
}

OptionValue::OptionValue(const std::string& string)
    : _type(Type::STRING)
    , _boolean(true)
    , _uint()
    , _string(string)
{
}

std::ostream& operator<<(std::ostream& os, const OptionValue& v)
{
    switch (v.type()) {
    case OptionValue::Type::BOOLEAN:
        os << (v ? "true" : "false");
        break;

    case OptionValue::Type::STRING:
        os << v.string();
        break;

    case OptionValue::Type::UNSIGNED:
        os << v.uint();
        break;
    }

    return os;
}

bool Argument::hasParameter() const
{
    switch (type) {
    case OptionType::FILENAME:
    case OptionType::STRING:
    case OptionType::UNSIGNED:
        return true;

    default:
        return false;
    }
}

const char* ::UnTech::CommandLine::optionString(const OptionType type)
{
    switch (type) {
    case OptionType::BOOLEAN:
    case OptionType::VERSION:
    case OptionType::HELP:
        return "boolean";

    case OptionType::STRING:
        return "string";

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
    , _filenames()
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
            _filenames.emplace_back(arg);
        }
    }

    // verify required switches
    for (const auto& a : _config.arguments) {
        if (a.required && _options[a.longName] == false) {
            if (a.shortName) {
                error(std::string("Missing ") + a.helpText
                      + " (-" + a.shortName + " / --" + a.longName + ")");
            }
            else {
                error(std::string("Missing ") + a.helpText
                      + " (--" + a.longName + ")");
            }
        }
    }

    // verify number of files
    if (_config.usesFiles) {
        if (_config.requireFile) {
            if (_filenames.size() == 0) {
                error("Expected " + _config.fileType);
            }
        }
        if (!_config.multipleFiles && _filenames.size() > 1) {
            error("Too many " + _config.fileType + " arguments");
        }
    }
    else {
        if (_filenames.size() != 0) {
            error("Unexpected argument " + _filenames.front());
        }
    }
}

bool Parser::parseShortSwitches(const char* currentArg, const char* nextArg)
{
    const char* arg = currentArg;

    if (*arg == 0) {
        error("Expected argument after -");
    }

    while (*arg) {
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
            error(std::string("Unknown argument -") + *arg);
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

    error(std::string("Unknown argument --") + arg);
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
    case OptionType::STRING:
        _options[argument.longName] = std::string(nextArg);
        return true;

    case OptionType::UNSIGNED: {
        auto n = String::toUnsigned(nextArg);
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
    std::cout << "Usage: " << _programExec << " [options]";

    if (_config.usesFiles) {
        std::cout << ' ';

        if (!_config.requireFile) {
            std::cout << "[";
        }
        std::cout << _config.fileType;
        if (_config.multipleFiles) {
            std::cout << "...";
        }
        if (!_config.requireFile) {
            std::cout << ']';
        }
    }

    for (const auto& a : _config.arguments) {
        int spacing = 24;

        std::cout << "\n\t";

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
        else if (a.type == OptionType::STRING) {
            std::cout << " <str>";
            spacing -= 6;
        }
        else if (a.type == OptionType::UNSIGNED) {
            std::cout << " <uint>";
            spacing -= 7;
        }

        if (!a.helpText.empty()) {
            for (int i = 0; i < spacing; i++) {
                std::cout << ' ';
            }
            std::cout << a.helpText;
        }

        if (a.defaultValue) {
            std::cout << " (default " << a.defaultValue << ")";
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
                 "\nThis program makes uses of the following third party source code:"
                 "\n\tLodePNG - Copyright (c) 2005-2016 Lode Vandevenne, zlib License"
                 "\n";
}

void Parser::error(const std::string& message)
{
    std::cerr << _programExec << ": " << message << std::endl;
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
