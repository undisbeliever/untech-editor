/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "version.h"
#include "models/common/string.h"
#include "models/common/type-traits.h"
#include "models/common/u8strings.h"
#include <algorithm>
#include <bitset>
#include <filesystem>
#include <iostream>
#include <span>
#include <string>
#include <tuple>

namespace UnTech::ArgParser_Impl {

template <class Config>
[[nodiscard]] static inline const typename Config::OutputT parseProgramArguments(const Config& config, int argc, const char** argv);

template <typename T>
[[nodiscard]] static inline auto parseArg(const std::string_view value, const std::string_view argLongName) -> T;
}

//
// Argument definitions
// ====================

namespace UnTech::ArgParser {

template <auto FieldPtr>
struct BooleanArg {
    static_assert(std::is_member_object_pointer_v<decltype(FieldPtr)>);
    static_assert(std::is_same_v<typename remove_member_pointer<decltype(FieldPtr)>::type, bool>);

    using OutputT = typename member_class<decltype(FieldPtr)>::type;
    using FieldT = bool;

    static constexpr bool HAS_PARAMETER = false;
    static constexpr bool REQUIRED_ARGUMENT = false;

    char shortName{};
    std::string_view longName;
    std::string_view helpText;

    static inline void setDefaultValue(OutputT&) {}

    static inline void parseArg(OutputT& output, const std::string_view)
    {
        output.*FieldPtr = true;
    }
};

template <auto FieldPtr>
struct RequiredArg {
    static_assert(std::is_member_object_pointer_v<decltype(FieldPtr)>);

    using OutputT = typename member_class<decltype(FieldPtr)>::type;
    using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    static constexpr bool HAS_PARAMETER = true;
    static constexpr bool REQUIRED_ARGUMENT = true;

    const char shortName{};
    const std::string_view longName;
    const std::string_view helpText;

    static inline void setDefaultValue(OutputT&) {}

    inline void parseArg(OutputT& output, const std::string_view value) const
    {
        output.*FieldPtr = ArgParser_Impl::parseArg<FieldT>(value, longName);
    }
};

template <auto FieldPtr>
struct OptionalArg {
    static_assert(std::is_member_object_pointer_v<decltype(FieldPtr)>);

    using OutputT = typename member_class<decltype(FieldPtr)>::type;
    using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    static constexpr bool HAS_PARAMETER = true;
    static constexpr bool REQUIRED_ARGUMENT = false;

    char shortName{};
    const std::string_view longName;
    const std::string_view helpText;
    const FieldT defaultValue{};

    inline void setDefaultValue(OutputT& output) const
    {
        output.*FieldPtr = defaultValue;
    }

    inline void parseArg(OutputT& output, const std::string_view value) const
    {
        output.*FieldPtr = ArgParser_Impl::parseArg<FieldT>(value, longName);
    }
};

template <auto FieldPtr>
    requires(std::is_same_v<typename remove_member_pointer<decltype(FieldPtr)>::type, std::filesystem::path>)
struct OptionalArg<FieldPtr> {
    static_assert(std::is_member_object_pointer_v<decltype(FieldPtr)>);

    using OutputT = typename member_class<decltype(FieldPtr)>::type;
    using FieldT = std::filesystem::path;

    static constexpr bool HAS_PARAMETER = true;
    static constexpr bool REQUIRED_ARGUMENT = false;

    char shortName{};
    std::string_view longName;
    std::string_view helpText;
    // no defaultValue - cannot constexpr a `std::filesystem::path`

    static inline void setDefaultValue(OutputT&) {}

    static inline void parseArg(OutputT& output, const std::string_view value)
    {
        output.*(FieldPtr) = std::filesystem::path{ value };
    }
};

template <class... T>
struct Config {
    using OutputT = typename std::tuple_element_t<0, std::tuple<T...>>::OutputT;
    static_assert((std::is_same_v<typename T::OutputT, OutputT> && ...), "All arguments must use the same OutputT");

    const std::string_view programName;
    const std::string_view inputFileType;

    const std::tuple<T...> arguments;
};

template <class... T>
[[nodiscard]] consteval static inline auto argParserConfig(const std::string_view programName, std::string_view inputFileName, T... arguments) -> auto
{
    return Config<T...>{ programName, inputFileName, std::make_tuple(arguments...) };
}

template <class Config>
[[nodiscard]] static inline auto parseProgramArguments(const Config& config, int argc, const char** argv) -> const typename Config::OutputT
{
    return ArgParser_Impl::parseProgramArguments(config, argc, argv);
}

}

namespace UnTech::ArgParser_Impl {

//
// Printing
// ========

// NOLINTBEGIN(clang-diagnostic-unneeded-internal-declaration)

[[noreturn]] static void error(const std::string_view msg)
{
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
}

[[noreturn]] static void error(const std::string_view msg1, const std::string_view msg2)
{
    std::cerr << msg1 << msg2 << std::endl;
    exit(EXIT_FAILURE);
}

[[noreturn]] static void error(const std::string_view msg1, const std::string_view msg2, const std::string_view msg3)
{
    std::cerr << msg1 << msg2 << msg3 << std::endl;
    exit(EXIT_FAILURE);
}

[[noreturn]] static auto duplicateArgumentError(const std::string_view longName) -> void
{
    error("Duplicate argument --", longName);
}

// NOLINTEND(clang-diagnostic-unneeded-internal-declaration)

template <class Config>
static inline auto printVersion(const Config& config) -> void
{
    std::cout << config.programName;

    stdout_write(UNTECH_VERSION_STRING);
    stdout_write(u8"\n");
    stdout_write(UNTECH_ABOUT_TEXT);
    stdout_write(u8"\n\n"
                 u8"This program makes use of Third Party Libraries:\n");
    stdout_write(THIRD_PARTY_LIBS);
}

//
// Template Magic
// ==============

template <class Config, typename UnaryFunction>
static inline auto for_each_argument(const Config& config, UnaryFunction f) -> void
{
    auto p = [&](auto&... a) {
        (f(a), ...);
    };
    std::apply(p, config.arguments);
}

template <class Config, typename BinaryFunction>
static inline auto for_each_enumerated_argument(const Config& config, BinaryFunction f) -> void
{
    size_t i = 0;
    auto p = [&](auto&... a) {
        (f(a, i++), ...);
    };
    std::apply(p, config.arguments);
}

template <class Config, typename BinaryPredicate>
[[nodiscard]] static inline auto for_each_argument_until_found(const Config& config, BinaryPredicate f) -> bool
{
    size_t i = 0;
    auto p = [&](auto&... a) {
        return (f(a, i++) || ...);
    };
    return std::apply(p, config.arguments);
}

//
// Help Test
// =========

template <class... T>
struct has_required_arg_tuple {
};

template <class... T>
struct has_required_arg_tuple<const std::tuple<T...>> {
    static constexpr bool value = (T::REQUIRED_ARGUMENT || ...);
};

template <class Config>
struct has_required_arg {
    static constexpr bool value = has_required_arg_tuple<decltype(Config::arguments)>::value;
};

template <class Arg>
struct argument_type_string {
};

template <>
struct argument_type_string<bool> {
    static constexpr std::string_view value{};
};

template <>
struct argument_type_string<unsigned> {
    static constexpr std::string_view value = " <uint>";
};

template <>
struct argument_type_string<std::filesystem::path> {
    static constexpr std::string_view value = " <file>";
};

template <class Arg>
static inline auto printDefaultValue(const Arg&) -> void
{
    // No default value
}

template <class Arg>
    requires requires { typename Arg::defaultValue; }
static inline auto printDefaultValue(const Arg& arg) -> void
{
    std::cout << " (default " << arg.defaultValue << ")";
}

template <class Arg>
static inline auto printArg(const Arg& arg) -> void
{
    constexpr std::string_view PADDING_STRING = "                        ";
    constexpr std::string_view PARAMETER_TYPE = argument_type_string<typename Arg::FieldT>::value;

    int padding = PADDING_STRING.size();

    std::cout << "\n  ";

    if (arg.shortName) {
        std::cout << '-' << arg.shortName << ' ';
        padding -= 3;
    }
    std::cout << "--" << arg.longName;
    padding -= arg.longName.size();

    if constexpr (not PARAMETER_TYPE.empty()) {
        std::cout << PARAMETER_TYPE;
        padding -= PARAMETER_TYPE.size();
    }

    if (padding > 0) {
        std::cout << PADDING_STRING.substr(0, padding);
    }

    std::cout << arg.helpText;

    printDefaultValue(arg);
}

template <class Config>
static inline auto printHelpText(const Config& config, const std::string_view execName) -> void
{
    // ::SHOULDO constexpr helpText::

    std::cout << config.programName
              << "\n\nUsage: " << execName << " [options] " << config.inputFileType;

    if constexpr (has_required_arg<Config>::value) {
        std::cout << "\n\nRequired arguments:";

        for_each_argument(config, [&](const auto& a) {
            constexpr bool REQUIRED_ARGUMENT = std::remove_reference_t<decltype(a)>::REQUIRED_ARGUMENT;
            if constexpr (REQUIRED_ARGUMENT) {
                printArg(a);
            }
        });
    }

    std::cout << "\n\nOptional arguments:";

    for_each_argument(config, [&](const auto& a) {
        constexpr bool REQUIRED_ARGUMENT = std::remove_reference_t<decltype(a)>::REQUIRED_ARGUMENT;
        if constexpr (not REQUIRED_ARGUMENT) {
            printArg(a);
        }
    });

    std::cout << "\n"
              << "\n  --version                 display version information"
                 "\n  -h --help                 display this help message"
              << std::endl;
}

//
// Default Output values
// =====================

template <class Config>
[[nodiscard]] static inline auto populateDefaults(const Config& config) -> typename Config::OutputT
{
    typename Config::OutputT output{};

    for_each_argument(config, [&](auto& a) { a.setDefaultValue(output); });

    return output;
}

//
// Argument Parsing
// ================

template <>
inline auto parseArg(const std::string_view value, const std::string_view argLongName) -> unsigned
{
    const auto n = String::toUint32(convert_old_string(value));
    if (!n) {
        error("invalid --", argLongName, " value (expected an integer)");
    }
    return n.value();
}

template <>
inline auto parseArg(const std::string_view value, const std::string_view /* argLongName */) -> std::filesystem::path
{
    return { value };
}

// Returns true if `nextArg` is used
template <class Config, size_t N>
[[nodiscard]] static inline auto parseShortSwitches(typename Config::OutputT& output, std::bitset<N>& argsEncountered,
                                                    const std::string_view execName, const Config& config, const std::string_view argument, const std::string_view nextArg) -> bool
{
    if (argument.empty()) {
        error("Expected argument after -");
    }

    bool usedNextArg = false;

    auto arg = argument;

    while (!arg.empty()) {
        bool missingParameter = false;
        bool consumedArg = false;

        const char argToTest = arg.front();

        if (argToTest == 'h') {
            printHelpText(config, execName);
            exit(EXIT_SUCCESS);
        }

        const bool found = for_each_argument_until_found(
            config,
            [&](const auto& a, const size_t i) -> bool {
                // Returns true if `a` matches `argToTest`.

                if (argToTest == a.shortName) {
                    if (argsEncountered.test(i)) {
                        duplicateArgumentError(a.longName);
                    }
                    argsEncountered.set(i);

                    constexpr bool HAS_PARAMETER = std::remove_reference_t<decltype(a)>::HAS_PARAMETER;

                    if constexpr (!HAS_PARAMETER) {
                        a.parseArg(output, {});
                        usedNextArg = false;
                    }
                    else {
                        if (arg.size() == 1) {
                            a.parseArg(output, nextArg);
                            usedNextArg = true;
                        }
                        else if (arg[1] == '=') {
                            a.parseArg(output, arg.substr(2));
                            consumedArg = true;
                            usedNextArg = false;
                        }
                        else {
                            missingParameter = true;
                        }
                    }
                    return true;
                }
                else {
                    return false;
                }
            });

        if (!found) {
            error("Unknown argument -", arg.substr(0, 1));
        }
        if (missingParameter) {
            error("Missing argument parameter for -", arg.substr(0, 1));
        }
        if (usedNextArg || consumedArg) {
            break;
        }

        arg.remove_prefix(1);
    }

    return usedNextArg;
}

// Returns true if `nextArg` is used
template <class Config, size_t N>
[[nodiscard]] static inline auto parseLongSwitch(typename Config::OutputT& output, std::bitset<N>& argsEncountered,
                                                 const std::string_view execName, const Config& config, const std::string_view argument, const std::string_view nextArg) -> bool
{
    using namespace std::literals;

    if (argument == "help"sv) {
        printHelpText(config, execName);
        exit(EXIT_SUCCESS);
    }
    if (argument == "version"sv) {
        printVersion(config);
        exit(EXIT_SUCCESS);
    }

    bool usedNextArg = false;

    const bool found = for_each_argument_until_found(
        config,
        [&](const auto& a, const size_t i) -> bool {
            // Returns true if the `a` matches `argument`.

            if (argument == a.longName) {
                if (argsEncountered.test(i)) {
                    duplicateArgumentError(a.longName);
                }
                argsEncountered.set(i);

                a.parseArg(output, nextArg);

                usedNextArg = std::remove_reference_t<decltype(a)>::HAS_PARAMETER;
                return true;
            }
            else {
                return false;
            }
        });

    if (!found) {
        error("Unknown argument --", argument);
    }

    return usedNextArg;
}

template <class Config, size_t N>
static inline auto checkForMissingArguments(const std::bitset<N>& argsEncountered, const typename Config::OutputT& output, const Config& config) -> void
{
    bool valid = true;

    if (output.inputFilename.empty()) {
        std::cerr << "Missing " << config.inputFileType << " argument\n";
        valid = false;
    }

    for_each_enumerated_argument(config, [&](const auto& a, const size_t i) {
        constexpr bool REQUIRED_ARGUMENT = std::remove_reference_t<decltype(a)>::REQUIRED_ARGUMENT;

        if constexpr (REQUIRED_ARGUMENT) {
            if (argsEncountered.test(i) == false) {
                std::cerr << "Missing --" << a.longName << " argument\n";
                valid = false;
            }
        }
    });

    if (!valid) {
        exit(EXIT_FAILURE);
    }
}

template <class Config>
[[nodiscard]] static inline auto parseProgramArguments(const Config& config, const std::string_view execName, std::span<const char*> arguments) -> const typename Config::OutputT
{
    constexpr size_t N_ARGS = std::tuple_size_v<decltype(config.arguments)>;
    static_assert(N_ARGS < 32);

    using namespace std::literals;

    typename Config::OutputT output = populateDefaults(config);

    bool inSwitches = true;

    std::bitset<N_ARGS> argsEncountered;

    for (size_t i = 0; i < arguments.size(); i++) {
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
                    useNextArg = parseLongSwitch(output, argsEncountered, execName, config, arg.substr(2), nextArg);
                }
            }
            else {
                useNextArg = parseShortSwitches(output, argsEncountered, execName, config, arg.substr(1), nextArg);
            }

            if (useNextArg) {
                i++;
            }
        }
        else {
            if (!output.inputFilename.empty()) {
                error("Too many ", config.inputFileType, " file arguments");
            }
            output.inputFilename = arg;
        }
    }

    checkForMissingArguments(argsEncountered, output, config);

    return output;
}

template <class Config>
[[nodiscard]] static inline auto parseProgramArguments(const Config& config, int argc, const char** argv) -> const typename Config::OutputT
{
    if (argc < 1 || argv == nullptr) {
        error("Invalid program arguments");
    }

    const auto span = std::span(argv, argc);

    // Confirm all argv elements are not null and not empty
    const bool valid = std::all_of(span.begin(), span.end(),
                                   [](const char* a) { return a && *a != 0; });
    if (not valid) {
        error("Invalid program arguments");
    }

    return parseProgramArguments(config, span[0], span.subspan(1));
}

}
