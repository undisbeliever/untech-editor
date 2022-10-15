/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <filesystem>
#include <list>
#include <map>
#include <span>
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
    // cppcheck-suppress noExplicitConstructor
    OptionValue(bool boolean = false);
    // cppcheck-suppress noExplicitConstructor
    OptionValue(unsigned uint);
    // cppcheck-suppress noExplicitConstructor
    OptionValue(std::filesystem::path&& p);

    [[nodiscard]] Type type() const { return _type; }
    operator bool() const { return _boolean; }
    [[nodiscard]] bool boolean() const { return _boolean; }
    [[nodiscard]] unsigned uint() const { return _uint; }
    [[nodiscard]] const std::filesystem::path& path() const { return _path; }
};

enum class OptionType {
    BOOLEAN,
    FILENAME,
    UNSIGNED,
    VERSION,
    HELP,
};

struct Argument {
    char shortName;
    std::string longName;
    OptionType type;
    bool required;
    OptionValue defaultValue;
    std::string helpText;

    [[nodiscard]] bool hasParameter() const;
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
    explicit Parser(const Config& config);

    // may exit application on help/version/error
    void parse(int argc, const char** const argv);

    void printHelpText() const;
    void printVersion() const;

    [[nodiscard]] const std::filesystem::path& inputFilename() const { return _inputFilename; }
    [[nodiscard]] const auto& options() const { return _options; }

private:
    void parse(const std::span<const char*> arguments);

    // all three return true if nextArg was used.
    bool parseShortSwitches(const std::string_view arg, const std::string_view nextArg);
    bool parseLongSwitch(const std::string_view arg, const std::string_view nextArg);
    bool parseSwitch(const Argument& argument, bool isShort, const std::string_view nextArg);

    template <typename... Args>
    void error(const Args... message) const;
    void error(const char* message, const Argument& argument, bool isShort) const;
};

}
