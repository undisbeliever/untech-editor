#pragma once

#include <list>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace UnTech {
namespace CommandLine {

class OptionValue {
public:
    enum class Type {
        BOOLEAN,
        STRING,
        UNSIGNED
    };

    OptionValue(bool boolean = false);
    OptionValue(const std::string& str);
    OptionValue(unsigned uint);
    OptionValue(const OptionValue&) = default;

    Type type() const { return _type; }
    operator bool() const { return _boolean; }
    bool boolean() const { return _boolean; }
    unsigned uint() const { return _uint; }
    const std::string& string() const { return _string; }

private:
    Type _type;
    bool _boolean;
    unsigned _uint;
    std::string _string;
};

enum class OptionType {
    BOOLEAN,
    FILENAME,
    STRING,
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
    bool usesFiles, requireFile, multipleFiles;
    std::string fileType;
    std::vector<Argument> arguments;
};

class Parser {
public:
    Parser(const Config& config);

    // may exit application on help/version/error
    void parse(int argc, const char* argv[]);

    void printHelpText();
    void printVersion();

    const auto& filenames() const { return _filenames; }
    const auto& options() const { return _options; }

private:
    // all three return true if nextArg was used.
    bool parseShortSwitches(const char* arg, const char* nextArg);
    bool parseLongSwitch(const char* arg, const char* nextArg);
    bool parseSwitch(const Argument& config, bool isShort, const char* nextArg);

    void error(const std::string& message);
    void error(const char* message, const Argument& config, bool isShort);

private:
    const Config& _config;
    std::string _programExec;

    std::list<std::string> _filenames;
    std::map<std::string, OptionValue> _options;
};
}
}
std::ostream& operator<<(std::ostream&, const UnTech::CommandLine::OptionValue&);