/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <filesystem>
#include <list>
#include <map>

#include <string>
#include <vector>

namespace UnTech::CommandLine {

class OptionValue {
public:
    enum class Type {
        BOOLEAN,
        FILENAME,
        UNSIGNED
    };

private:
    Type _type;
    bool _boolean;
    unsigned _uint;
    std::filesystem::path _path;

public:
    OptionValue(bool boolean = false);
    OptionValue(unsigned uint);
    OptionValue(std::filesystem::path&& p);

    Type type() const { return _type; }
    operator bool() const { return _boolean; }
    bool boolean() const { return _boolean; }
    unsigned uint() const { return _uint; }
    const std::filesystem::path& path() const { return _path; }
};

enum class OptionType {
    BOOLEAN,
    FILENAME,
    UNSIGNED,
    VERSION,
    HELP,
};
const char* optionString(const OptionType type);

struct Argument {
    char shortName;
    std::string longName;
    OptionType type;
    bool required;
    OptionValue defaultValue;
    std::string helpText;

    bool hasParameter() const;
};

struct Config {
    std::string programName;
    std::string inputFileType;
    std::vector<Argument> arguments;
};

class Parser {

private:
    const Config& _config;
    std::string _programExec;

    std::filesystem::path _inputFilename;
    std::map<std::string, OptionValue> _options;

public:
    Parser(const Config& config);

    // may exit application on help/version/error
    void parse(int argc, const char* argv[]);

    void printHelpText() const;
    void printVersion() const;

    const std::filesystem::path& inputFilename() const { return _inputFilename; }
    const auto& options() const { return _options; }

private:
    // all three return true if nextArg was used.
    bool parseShortSwitches(const char* arg, const char* nextArg);
    bool parseLongSwitch(const char* arg, const char* nextArg);
    bool parseSwitch(const Argument& config, bool isShort, const char* nextArg);

    template <typename... Args>
    void error(const Args... message) const;
    void error(const char* message, const Argument& config, bool isShort) const;
};

}
