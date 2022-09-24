// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <cli11-yaml/cli11-yaml.hpp>

#include <cstdio>
#include <sstream>

using Catch::Matchers::ContainsSubstring;

namespace CLI {
inline bool
operator==(const CLI::ConfigItem& lh, const CLI::ConfigItem& rh) {
    return lh.parents == rh.parents
            && lh.name == rh.name
            && lh.inputs == rh.inputs;
}

inline std::ostream&
operator<<(std::ostream& os, const ConfigItem& item) {
    auto join = [&os](const char* name, const std::vector<std::string>& v) {
        if (!v.empty()) {
            os << "\n - " << name << ": [";
            for (size_t i = 0; i < v.size(); ++i) {
                os << (i ? ", " : "") << v[i];
            }
            os << "]";
        }
    };

    os << "[";
    join("parents", item.parents);
    os << "\n - name: " << item.name;
    join("inputs", item.inputs);
    os << "\n]";
    return os;
}

}

TEST_CASE("Yaml: StringBased: First", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream {
                    "one=three\n"
                    "two=four\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{
                    "one: three\n"
                    "two: four\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: FirstWithComments", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{
                ";this is a comment\n"
                "one=three\n"
                "two=four\n"
                "; and another one\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"#this is a comment\n"
                   "one: three\n"
                   "two: four\n"
                   "# and another one\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: Quotes", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{R"(one = "three")" "\n"
                   R"(two = 'four')" "\n"
                   R"(five = "six and seven")" "\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{R"(one: "three")" "\n"
                   R"(two: 'four')" "\n"
                   R"(five: "six and seven")" "\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: Vector", "[config]")
{
    auto outputInit = CLI::ConfigINI().from_config(
            Stream{"one = three\n"
                   "two = four\n"
                   "five = six and seven\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"one: three\n"
                   "two: four\n"
                   "five:\n"
                   "  - six\n"
                   "  - and\n"
                   "  - seven\n"
            });

    auto outputYamlBracket = CLI::ConfigYAML().from_config(
            Stream{"one: three\n"
                   "two: four\n"
                   "five: [six, and, seven]\n"
            });

    CHECK(outputInit == outputYaml);
    CHECK(outputInit == outputYamlBracket);
}

TEST_CASE("Yaml: StringBased: TomlVector", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"one = [three]\n"
                   "two = [four]\n"
                   "five = [six, and, seven]\n"
                   "eight = [nine, \n"
                            "ten, eleven,     twelve    \n"
                            "]\n"
                    "one_more = [one, \n"
                                "two,     three  ]    \n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"one: [three]\n"
                   "two: [four]\n"
                   "five: [six, and, seven]\n"
                   "eight: [nine, \n"
                          "  ten, eleven,     twelve    \n"
                          "  ]\n"
                    "one_more: [one, \n"
                              "  two,     three  ]    \n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: Spaces", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"one = three\n"
                   "two = four"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"one : three\n"
                   "two : four"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: Sections", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"one=three\n"
                   "[second]\n"
                   "  two=four\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"one: three\n"
                   "second:\n"
                   "  two: four\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: SpacesSections", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"one=three\n\n"
                   "[second]   \n"
                   "   \n"
                   "  two=four\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"one: three\n\n"
                   "second:   \n"
                   "   \n"
                   "  two: four\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: Layers", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"simple = true\n\n"
                   "[other]\n"
                   "[other.sub2]\n"
                   "[other.sub2.sub-level2]\n"
                   "[other.sub2.sub-level2.sub-level3]\n"
                   "absolute_newest = true\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"simple: true\n\n"
                   "other:\n"
                   "  sub2:\n"
                   "    sub-level2:\n"
                   "      sub-level3:\n"
                   "        absolute_newest: true\n"
            });

    // 2 flags and 4 openings and 4 closings
    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: LayersSkip", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"simple = true\n\n"
                   "[other.sub2]\n"
                   "[other.sub2.sub-level2.sub-level3]\n"
                   "absolute_newest = true\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"simple : true\n\n"
                   "other:\n"
                   "  sub2:\n"
                   "\n"
                   "other:\n"
                   "  sub2:\n"
                   "    sub-level2:\n"
                   "      sub-level3:\n"
                   "        absolute_newest : true\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: LayersSkipOrdered", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"simple = true\n\n"
                   "[other.sub2.sub-level2.sub-level3]\n"
                   "[other.sub2]\n"
                   "absolute_newest = true\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"simple: true\n\n"
                   "other:\n"
                   "  sub2:\n"
                   "    sub-level2:\n"
                   "      sub-level3:\n"
                   "other:\n"
                   "  sub2:\n"
                   "    absolute_newest: true\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: LayersChange", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"simple = true\n\n"
                   "[other.sub2]\n"
                   "[other.sub3]\n"
                   "absolute_newest = true\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"simple: true\n\n"
                   "other:\n"
                   "  sub2:\n"
                   "other:\n"
                   "  sub3:\n"
                   "    absolute_newest: true\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: Layers2LevelChange", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"simple = true\n\n"
                   "[other.sub2.cmd]\n"
                   "[other.sub3.cmd]\n"
                   "absolute_newest = true\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"simple: true\n\n"
                   "other:\n"
                   "  sub2:\n"
                   "    cmd:\n"
                   "  sub3:\n"
                   "    cmd:\n"
                   "      absolute_newest: true\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: Layers3LevelChange", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"[other.sub2.subsub.cmd]\n"
                   "[other.sub3.subsub.cmd]\n"
                   "absolute_newest = true\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"other:\n"
                   "  sub2:\n"
                   "    subsub:\n"
                   "      cmd:\n"
                   "  sub3:\n"
                   "    subsub:\n"
                   "      cmd:\n"
                   "        absolute_newest: true\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: newSegment", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"[other.sub2.subsub.cmd]\n"
                   "flag = true\n"
                   "[another]\n"
                   "absolute_newest = true\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"other:\n"
                   "  sub2:\n"
                   "    subsub:\n"
                   "      cmd:\n"
                   "        flag: true\n"
                   "another:\n"
                   "  absolute_newest: true\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: LayersDirect", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"simple = true\n\n"
                   "[other.sub2.sub-level2.sub-level3]\n"
                   "absolute_newest = true\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"simple: true\n\n"
                   "other:\n"
                   "  sub2:\n"
                   "    sub-level2:\n"
                   "      sub-level3:\n"
                   "        absolute_newest: true\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: LayersComplex", "[config]")
{
    auto outputIni = CLI::ConfigINI().from_config(
            Stream{"simple = true\n\n"
                   "[other.sub2.sub-level2.sub-level3]\n"
                   "absolute_newest = true\n"
                   "[other.sub2.sub-level2]\n"
                   "still_newer = true\n"
                   "[other.sub2]\n"
                   "newest = true\n"
            });

    auto outputYaml = CLI::ConfigYAML().from_config(
            Stream{"simple: true\n\n"
                   "other:\n"
                   "  sub2:\n"
                   "    sub-level2:\n"
                   "      sub-level3:\n"
                   "        absolute_newest: true\n"
                   "      still_newer: true\n"
                   "    newest: true\n"
            });

    CHECK(outputIni == outputYaml);
}

TEST_CASE("Yaml: StringBased: file_error", "[config]")
{
    CHECK_THROWS_AS(CLI::ConfigYAML().from_file("nonexist_file"), CLI::FileError);
}

TEST_CASE_METHOD(TApp, "YamlNotRequired", "[config]")
{
    TempFile tmpYaml{"TestYamlTmp.yaml"};

    app.set_config("--config", tmpYaml);
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    {
        std::ofstream out{tmpYaml};
        out << "two: 99" << std::endl;
        out << "three: 3" << std::endl;
    }

    int one = 0, two = 0, three = 0;
    app.add_option("--one", one);
    app.add_option("--two", two);
    app.add_option("--three", three);

    args = {"--one=1"};

    run();

    CHECK(one == 1);
    CHECK(two == 99);
    CHECK(three == 3);

    one = two = three = 0;
    args = {"--one=1", "--two=2"};

    run();

    CHECK(one == 1);
    CHECK(two == 2);
    CHECK(three == 3);
    CHECK("TestYamlTmp.yaml" == app["--config"]->as<std::string>());
}

TEST_CASE_METHOD(TApp, "YamlSuccessOnUnknownOption", "[config]")
{
    TempFile tmpYaml{"TestYamlTmp.yaml"};

    app.set_config("--config", tmpYaml);
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.allow_config_extras(true);

    {
        std::ofstream out{tmpYaml};
        out << "three: 3" << std::endl;
        out << "two: 99" << std::endl;
    }

    int two{0};
    app.add_option("--two", two);
    run();
    CHECK(two == 99);
}

TEST_CASE_METHOD(TApp, "YamlGetRemainingOption", "[config]")
{
    TempFile tmpYaml{"TestYamlTmp.yaml"};

    app.set_config("--config", tmpYaml);
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.allow_config_extras(true);

    std::string ExtraOption = "three";
    std::string ExtraOptionValue = "3";
    {
        std::ofstream out{tmpYaml};
        out << ExtraOption << ": " << ExtraOptionValue << std::endl;
        out << "two: 99" << std::endl;
    }

    int two{0};
    app.add_option("--two", two);
    REQUIRE_NOTHROW(run());
    std::vector<std::string> ExpectedRemaining = {ExtraOption};
    CHECK(ExpectedRemaining == app.remaining());
}

TEST_CASE_METHOD(TApp, "YamlGetNoRemaining", "[config]")
{
    TempFile tmpYaml{"TestYamlTmp.yaml"};

    app.set_config("--config", tmpYaml);
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.allow_config_extras(true);

    {
        std::ofstream out{tmpYaml};
        out << "two: 99" << std::endl;
    }

    int two{0};
    app.add_option("--two", two);
    REQUIRE_NOTHROW(run());
    CHECK(app.remaining().empty());
}

TEST_CASE_METHOD(TApp, "YamlRequiredNoDefault", "[config]")
{
    app.set_config("--config")->required();
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    int two{0};
    app.add_option("--two", two);
    REQUIRE_THROWS_AS(run(), CLI::FileError);
    // test to make sure help still gets called correctly
    // GitHub issue #533 https://github.com/CLIUtils/CLI11/issues/553
    args = {"--help"};
    REQUIRE_THROWS_AS(run(), CLI::CallForHelp);
}

TEST_CASE_METHOD(TApp, "YamlNotRequiredNoDefault", "[config]")
{

    app.set_config("--config");
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    int two{0};
    app.add_option("--two", two);
    REQUIRE_NOTHROW(run());
}

TEST_CASE_METHOD(TApp, "YamlNotRequiredNotDefault", "[config]")
{

    TempFile tmpYaml{"TestYamlTmp.yaml"};
    TempFile tmpYaml2{"TestYamlTmp2.yaml"};

    app.set_config("--config", tmpYaml);
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    {
        std::ofstream out{tmpYaml};
        out << "two: 99" << std::endl;
        out << "three: 3" << std::endl;
    }

    {
        std::ofstream out{tmpYaml2};
        out << "two: 98" << std::endl;
        out << "three: 4" << std::endl;
    }

    int one{0}, two{0}, three{0};
    app.add_option("--one", one);
    app.add_option("--two", two);
    app.add_option("--three", three);

    run();
    CHECK(tmpYaml.c_str() == app["--config"]->as<std::string>());
    CHECK(two == 99);
    CHECK(three == 3);

    args = {"--config", tmpYaml2};
    run();

    CHECK(two == 98);
    CHECK(three == 4);
    CHECK(tmpYaml2.c_str() == app.get_config_ptr()->as<std::string>());
}

//TEST_CASE_METHOD(TApp, "Yaml: MultiConfig", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//    TempFile tmpYaml2{"TestYamlTmp2.yaml"};
//
//    app.set_config("--config")->expected(1, 3);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=99" << std::endl;
//        out << "three=3" << std::endl;
//    }
//
//    {
//        std::ofstream out{tmpYaml2};
//        out << "[default]" << std::endl;
//        out << "one=55" << std::endl;
//        out << "three=4" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0};
//    app.add_option("--one", one);
//    app.add_option("--two", two);
//    app.add_option("--three", three);
//
//    args = {"--config", tmpYaml2, "--config", tmpYaml};
//    run();
//
//    CHECK(two == 99);
//    CHECK(three == 3);
//    CHECK(one == 55);
//
//    args = {"--config", tmpYaml, "--config", tmpYaml2};
//    run();
//
//    CHECK(two == 99);
//    CHECK(three == 4);
//    CHECK(one == 55);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: MultiConfig_single", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//    TempFile tmpYaml2{"TestYamlTmp2.yaml"};
//
//    app.set_config("--config")->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=99" << std::endl;
//        out << "three=3" << std::endl;
//    }
//
//    {
//        std::ofstream out{tmpYaml2};
//        out << "[default]" << std::endl;
//        out << "one=55" << std::endl;
//        out << "three=4" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0};
//    app.add_option("--one", one);
//    app.add_option("--two", two);
//    app.add_option("--three", three);
//
//    args = {"--config", tmpYaml2, "--config", tmpYaml};
//    run();
//
//    CHECK(two == 99);
//    CHECK(three == 3);
//    CHECK(one == 0);
//
//    two = 0;
//    args = {"--config", tmpYaml, "--config", tmpYaml2};
//    run();
//
//    CHECK(two == 0);
//    CHECK(three == 4);
//    CHECK(one == 55);
//}
//
//TEST_CASE_METHOD(TApp, "YamlRequiredNotFound", "[config]")
//{
//
//    std::string noini = "TestIniNotExist.ini";
//    app.set_config("--config", noini, "", true);
//
//    CHECK_THROWS_AS(run(), CLI::FileError);
//}
//
//TEST_CASE_METHOD(TApp, "YamlNotRequiredPassedNotFound", "[config]")
//{
//
//    std::string noini = "TestIniNotExist.ini";
//    app.set_config("--config", "", "", false);
//
//    args = {"--config", noini};
//    CHECK_THROWS_AS(run(), CLI::FileError);
//}
//
//TEST_CASE_METHOD(TApp, "YamlOverwrite", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=99" << std::endl;
//    }
//
//    std::string orig = "filename_not_exist.ini";
//    std::string next = "TestIniTmp.ini";
//    app.set_config("--config", orig);
//    // Make sure this can be overwritten
//    app.set_config("--conf", next);
//    int two{7};
//    app.add_option("--two", two);
//
//    run();
//
//    CHECK(two == 99);
//}
//
//TEST_CASE_METHOD(TApp, "YamlRequired", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml, "", true);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=99" << std::endl;
//        out << "three=3" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0};
//    app.add_option("--one", one)->required();
//    app.add_option("--two", two)->required();
//    app.add_option("--three", three)->required();
//
//    args = {"--one=1"};
//
//    run();
//    CHECK(1 == one);
//    CHECK(99 == two);
//    CHECK(3 == three);
//
//    one = two = three = 0;
//    args = {"--one=1", "--two=2"};
//
//    CHECK_NOTHROW(run());
//    CHECK(1 == one);
//    CHECK(2 == two);
//    CHECK(3 == three);
//
//    args = {};
//
//    CHECK_THROWS_AS(run(), CLI::RequiredError);
//
//    args = {"--two=2"};
//
//    CHECK_THROWS_AS(run(), CLI::RequiredError);
//}
//
//TEST_CASE_METHOD(TApp, "YamlInlineComment", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml, "", true);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=99 ; this is a two" << std::endl;
//        out << "three=3; this is a three" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0};
//    app.add_option("--one", one)->required();
//    app.add_option("--two", two)->required();
//    app.add_option("--three", three)->required();
//
//    args = {"--one=1"};
//
//    run();
//    CHECK(1 == one);
//    CHECK(99 == two);
//    CHECK(3 == three);
//
//    one = two = three = 0;
//    args = {"--one=1", "--two=2"};
//
//    CHECK_NOTHROW(run());
//    CHECK(1 == one);
//    CHECK(2 == two);
//    CHECK(3 == three);
//
//    args = {};
//
//    CHECK_THROWS_AS(run(), CLI::RequiredError);
//
//    args = {"--two=2"};
//
//    CHECK_THROWS_AS(run(), CLI::RequiredError);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlInlineComment", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml, "", true);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=99 # this is a two" << std::endl;
//        out << "three=3# this is a three" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0};
//    app.add_option("--one", one)->required();
//    app.add_option("--two", two)->required();
//    app.add_option("--three", three)->required();
//
//    args = {"--one=1"};
//
//    run();
//    CHECK(1 == one);
//    CHECK(99 == two);
//    CHECK(3 == three);
//
//    one = two = three = 0;
//    args = {"--one=1", "--two=2"};
//
//    CHECK_NOTHROW(run());
//    CHECK(1 == one);
//    CHECK(2 == two);
//    CHECK(3 == three);
//
//    args = {};
//
//    CHECK_THROWS_AS(run(), CLI::RequiredError);
//
//    args = {"--two=2"};
//
//    CHECK_THROWS_AS(run(), CLI::RequiredError);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: ConfigModifiers", "[config]")
//{
//
//    app.set_config("--config", "test.ini", "", true);
//
//    auto cfgptr = app.get_config_formatter_base();
//
//    cfgptr->section("test");
//    CHECK(cfgptr->section() == "test");
//
//    CHECK(cfgptr->sectionRef() == "test");
//    auto& sref = cfgptr->sectionRef();
//    sref = "this";
//    CHECK(cfgptr->section() == "this");
//
//    cfgptr->index(5);
//    CHECK(cfgptr->index() == 5);
//
//    CHECK(cfgptr->indexRef() == 5);
//    auto& iref = cfgptr->indexRef();
//    iref = 7;
//    CHECK(cfgptr->index() == 7);
//}
//
//TEST_CASE_METHOD(TApp, "YamlVector", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=2 3" << std::endl;
//        out << "three=1 2 3" << std::endl;
//    }
//
//    std::vector<int> two, three;
//    app.add_option("--two", two)->expected(2)->required();
//    app.add_option("--three", three)->required();
//
//    run();
//
//    CHECK(two == std::vector<int>({2, 3}));
//    CHECK(three == std::vector<int>({1, 2, 3}));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TOMLVector", "[config]")
//{
//
//    TempFile tmptoml{"TestTomlTmp.toml"};
//
//    app.set_config("--config", tmptoml);
//
//    {
//        std::ofstream out{tmptoml};
//        out << "#this is a comment line\n";
//        out << "[default]\n";
//        out << "two=[2,3]\n";
//        out << "three=[1,2,3]\n";
//    }
//
//    std::vector<int> two, three;
//    app.add_option("--two", two)->expected(2)->required();
//    app.add_option("--three", three)->required();
//
//    run();
//
//    CHECK(two == std::vector<int>({2, 3}));
//    CHECK(three == std::vector<int>({1, 2, 3}));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: ColonValueSep", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "#this is a comment line\n";
//        out << "[default]\n";
//        out << "two:2\n";
//        out << "three:3\n";
//    }
//
//    int two{0}, three{0};
//    app.add_option("--two", two);
//    app.add_option("--three", three);
//
//    app.get_config_formatter_base()->valueSeparator(':');
//
//    run();
//
//    CHECK(two == 2);
//    CHECK(three == 3);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TOMLVectordirect", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    app.config_formatter(std::make_shared<CLI::ConfigTOML>());
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "#this is a comment line\n";
//        out << "[default]\n";
//        out << "two=[2,3]\n";
//        out << "three=[1,2,3]\n";
//    }
//
//    std::vector<int> two, three;
//    app.add_option("--two", two)->expected(2)->required();
//    app.add_option("--three", three)->required();
//
//    run();
//
//    CHECK(two == std::vector<int>({2, 3}));
//    CHECK(three == std::vector<int>({1, 2, 3}));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TOMLStringVector", "[config]")
//{
//
//    TempFile tmptoml{"TestTomlTmp.toml"};
//
//    app.set_config("--config", tmptoml);
//
//    {
//        std::ofstream out{tmptoml};
//        out << "#this is a comment line\n";
//        out << "[default]\n";
//        out << "zero1=[]\n";
//        out << "zero2={}\n";
//        out << "zero3={}\n";
//        out << "nzero={}\n";
//        out << "one=[\"1\"]\n";
//        out << "two=[\"2\",\"3\"]\n";
//        out << "three=[\"1\",\"2\",\"3\"]\n";
//    }
//
//    std::vector<std::string> nzero, zero1, zero2, zero3, one, two, three;
//    app.add_option("--zero1", zero1)->required()->expected(0, 99)->default_str("{}");
//    app.add_option("--zero2", zero2)->required()->expected(0, 99)->default_val(std::vector<std::string>{});
//    // if no default is specified the argument results in an empty string
//    app.add_option("--zero3", zero3)->required()->expected(0, 99);
//    app.add_option("--nzero", nzero)->required();
//    app.add_option("--one", one)->required();
//    app.add_option("--two", two)->required();
//    app.add_option("--three", three)->required();
//
//    run();
//
//    CHECK(zero1 == std::vector<std::string>({}));
//    CHECK(zero2 == std::vector<std::string>({}));
//    CHECK(zero3 == std::vector<std::string>({""}));
//    CHECK(nzero == std::vector<std::string>({"{}"}));
//    CHECK(one == std::vector<std::string>({"1"}));
//    CHECK(two == std::vector<std::string>({"2", "3"}));
//    CHECK(three == std::vector<std::string>({"1", "2", "3"}));
//}
//
//TEST_CASE_METHOD(TApp, "YamlVectorCsep", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "#this is a comment line\n";
//        out << "[default]\n";
//        out << "zero1=[]\n";
//        out << "zero2=[]\n";
//        out << "one=[1]\n";
//        out << "two=[2,3]\n";
//        out << "three=1,2,3\n";
//    }
//
//    std::vector<int> zero1, zero2, one, two, three;
//    app.add_option("--zero1", zero1)->required()->expected(0, 99)->default_str("{}");
//    app.add_option("--zero2", zero2)->required()->expected(0, 99)->default_val(std::vector<int>{});
//    app.add_option("--one", one)->required();
//    app.add_option("--two", two)->expected(2)->required();
//    app.add_option("--three", three)->required();
//
//    run();
//
//    CHECK(zero1 == std::vector<int>({}));
//    CHECK(zero2 == std::vector<int>({}));
//    CHECK(one == std::vector<int>({1}));
//    CHECK(two == std::vector<int>({2, 3}));
//    CHECK(three == std::vector<int>({1, 2, 3}));
//}
//
//TEST_CASE_METHOD(TApp, "YamlVectorMultiple", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "#this is a comment line\n";
//        out << "[default]\n";
//        out << "two=2\n";
//        out << "two=3\n";
//        out << "three=1\n";
//        out << "three=2\n";
//        out << "three=3\n";
//    }
//
//    std::vector<int> two, three;
//    app.add_option("--two", two)->expected(2)->required();
//    app.add_option("--three", three)->required();
//
//    run();
//
//    CHECK(two == std::vector<int>({2, 3}));
//    CHECK(three == std::vector<int>({1, 2, 3}));
//}
//
//TEST_CASE_METHOD(TApp, "YamlLayered", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[subcom]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "subsubcom.val=3" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0};
//    app.add_option("--val", one);
//    auto* subcom = app.add_subcommand("subcom");
//    subcom->add_option("--val", two);
//    auto* subsubcom = subcom->add_subcommand("subsubcom");
//    subsubcom->add_option("--val", three);
//
//    run();
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//    CHECK(three == 3);
//
//    CHECK(0U == subcom->count());
//    CHECK(!*subcom);
//}
//
//TEST_CASE_METHOD(TApp, "YamlLayeredStream", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[subcom]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "subsubcom.val=3" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0};
//    app.add_option("--val", one);
//    auto* subcom = app.add_subcommand("subcom");
//    subcom->add_option("--val", two);
//    auto* subsubcom = subcom->add_subcommand("subsubcom");
//    subsubcom->add_option("--val", three);
//
//    std::ifstream in{tmpYaml};
//    app.parse_from_stream(in);
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//    CHECK(three == 3);
//
//    CHECK(0U == subcom->count());
//    CHECK(!*subcom);
//}
//
//TEST_CASE_METHOD(TApp, "YamlLayeredDotSection", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[subcom]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "[subcom.subsubcom]" << std::endl;
//        out << "val=3" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0};
//    app.add_option("--val", one);
//    auto* subcom = app.add_subcommand("subcom");
//    subcom->add_option("--val", two);
//    auto* subsubcom = subcom->add_subcommand("subsubcom");
//    subsubcom->add_option("--val", three);
//
//    run();
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//    CHECK(three == 3);
//
//    CHECK(0U == subcom->count());
//    CHECK(!*subcom);
//
//    three = 0;
//    // check maxlayers
//    app.get_config_formatter_base()->maxLayers(1);
//    run();
//    CHECK(three == 0);
//}
//
//TEST_CASE_METHOD(TApp, "YamlLayeredCustomSectionSeparator", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[subcom]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "[subcom|subsubcom]" << std::endl;
//        out << "val=3" << std::endl;
//    }
//    app.get_config_formatter_base()->parentSeparator('|');
//    int one{0}, two{0}, three{0};
//    app.add_option("--val", one);
//    auto* subcom = app.add_subcommand("subcom");
//    subcom->add_option("--val", two);
//    auto* subsubcom = subcom->add_subcommand("subsubcom");
//    subsubcom->add_option("--val", three);
//
//    run();
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//    CHECK(three == 3);
//
//    CHECK(0U == subcom->count());
//    CHECK(!*subcom);
//}
//
//TEST_CASE_METHOD(TApp, "YamlLayeredOptionGroupAlias", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[ogroup]" << std::endl;
//        out << "val2=2" << std::endl;
//    }
//    int one{0}, two{0};
//    app.add_option("--val", one);
//    auto* subcom = app.add_option_group("ogroup")->alias("ogroup");
//    subcom->add_option("--val2", two);
//
//    run();
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//}
//
//TEST_CASE_METHOD(TApp, "YamlSubcommandConfigurable", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[subcom]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "subsubcom.val=3" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0};
//    app.add_option("--val", one);
//    auto* subcom = app.add_subcommand("subcom");
//    subcom->configurable();
//    subcom->add_option("--val", two);
//    auto* subsubcom = subcom->add_subcommand("subsubcom");
//    subsubcom->add_option("--val", three);
//
//    run();
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//    CHECK(three == 3);
//
//    CHECK(1U == subcom->count());
//    CHECK(*subcom);
//    CHECK(app.got_subcommand(subcom));
//}
//
//TEST_CASE_METHOD(TApp, "YamlSubcommandConfigurablePreParse", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[subcom]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "subsubcom.val=3" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0}, four{0};
//    app.add_option("--val", one);
//    auto* subcom = app.add_subcommand("subcom");
//    auto* subcom2 = app.add_subcommand("subcom2");
//    subcom->configurable();
//    std::vector<std::size_t> parse_c;
//    subcom->preparse_callback([&parse_c](std::size_t cnt) { parse_c.push_back(cnt); });
//    subcom->add_option("--val", two);
//    subcom2->add_option("--val", four);
//    subcom2->preparse_callback([&parse_c](std::size_t cnt) { parse_c.push_back(cnt + 2623); });
//    auto* subsubcom = subcom->add_subcommand("subsubcom");
//    subsubcom->add_option("--val", three);
//
//    run();
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//    CHECK(three == 3);
//    CHECK(four == 0);
//
//    CHECK(1U == parse_c.size());
//    CHECK(2U == parse_c[0]);
//
//    CHECK(0U == subcom2->count());
//}
//
//TEST_CASE_METHOD(TApp, "YamlSection", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//    app.get_config_formatter_base()->section("config");
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[config]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "subsubcom.val=3" << std::endl;
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//    }
//
//    int val{0};
//    app.add_option("--val", val);
//
//    run();
//
//    CHECK(2 == val);
//}
//
//TEST_CASE_METHOD(TApp, "YamlSection2", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//    app.get_config_formatter_base()->section("config");
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[config]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "subsubcom.val=3" << std::endl;
//    }
//
//    int val{0};
//    app.add_option("--val", val);
//
//    run();
//
//    CHECK(2 == val);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: jsonLikeParsing", "[config]")
//{
//
//    TempFile tmpjson{"TestJsonTmp.json"};
//
//    app.set_config("--config", tmpjson);
//    app.get_config_formatter_base()->valueSeparator(':');
//
//    {
//        std::ofstream out{tmpjson};
//        out << "{" << std::endl;
//        out << "\"val\":1," << std::endl;
//        out << R"("val2":"test",)" << std::endl;
//        out << "\"flag\":true" << std::endl;
//        out << "}" << std::endl;
//    }
//
//    int val{0};
//    app.add_option("--val", val);
//    std::string val2{0};
//    app.add_option("--val2", val2);
//
//    bool flag{false};
//    app.add_flag("--flag", flag);
//
//    run();
//
//    CHECK(1 == val);
//    CHECK(val2 == "test");
//    CHECK(flag);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlSectionNumber", "[config]")
//{
//
//    TempFile tmpYaml{"TestTomlTmp.toml"};
//
//    app.set_config("--config", tmpYaml);
//    app.get_config_formatter_base()->section("config")->index(0);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[[config]]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "subsubcom.val=3" << std::endl;
//        out << "[[config]]" << std::endl;
//        out << "val=4" << std::endl;
//        out << "subsubcom.val=3" << std::endl;
//        out << "[[config]]" << std::endl;
//        out << "val=6" << std::endl;
//        out << "subsubcom.val=3" << std::endl;
//    }
//
//    int val{0};
//    app.add_option("--val", val);
//
//    run();
//
//    CHECK(2 == val);
//
//    auto& index = app.get_config_formatter_base()->indexRef();
//    index = 1;
//    run();
//
//    CHECK(4 == val);
//
//    index = -1;
//    run();
//    // Take the first section in this case
//    CHECK(2 == val);
//    index = 2;
//    run();
//
//    CHECK(6 == val);
//}
//
//TEST_CASE_METHOD(TApp, "YamlSubcommandConfigurableParseComplete", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[subcom]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "[subcom.subsubcom]" << std::endl;
//        out << "val=3" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0}, four{0};
//    app.add_option("--val", one);
//    auto* subcom = app.add_subcommand("subcom");
//    auto* subcom2 = app.add_subcommand("subcom2");
//    subcom->configurable();
//    std::vector<std::size_t> parse_c;
//    subcom->parse_complete_callback([&parse_c]() { parse_c.push_back(58); });
//    subcom->add_option("--val", two);
//    subcom2->add_option("--val", four);
//    subcom2->parse_complete_callback([&parse_c]() { parse_c.push_back(2623); });
//    auto* subsubcom = subcom->add_subcommand("subsubcom");
//    // configurable should be inherited
//    subsubcom->parse_complete_callback([&parse_c]() { parse_c.push_back(68); });
//    subsubcom->add_option("--val", three);
//
//    run();
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//    CHECK(three == 3);
//    CHECK(four == 0);
//
//    REQUIRE(2u == parse_c.size());
//    CHECK(68U == parse_c[0]);
//    CHECK(58U == parse_c[1]);
//    CHECK(1u == subsubcom->count());
//    CHECK(0u == subcom2->count());
//}
//
//TEST_CASE_METHOD(TApp, "YamlSubcommandMultipleSections", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//        out << "[subcom]" << std::endl;
//        out << "val=2" << std::endl;
//        out << "[subcom.subsubcom]" << std::endl;
//        out << "val=3" << std::endl;
//        out << "[subcom2]" << std::endl;
//        out << "val=4" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0}, four{0};
//    app.add_option("--val", one);
//    auto* subcom = app.add_subcommand("subcom");
//    auto* subcom2 = app.add_subcommand("subcom2");
//    subcom->configurable();
//    std::vector<std::size_t> parse_c;
//    subcom->parse_complete_callback([&parse_c]() { parse_c.push_back(58); });
//    subcom->add_option("--val", two);
//    subcom2->add_option("--val", four);
//    subcom2->parse_complete_callback([&parse_c]() { parse_c.push_back(2623); });
//    subcom2->configurable(false);
//    auto* subsubcom = subcom->add_subcommand("subsubcom");
//    // configurable should be inherited
//    subsubcom->parse_complete_callback([&parse_c]() { parse_c.push_back(68); });
//    subsubcom->add_option("--val", three);
//
//    run();
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//    CHECK(three == 3);
//    CHECK(four == 4);
//
//    REQUIRE(2u == parse_c.size());
//    CHECK(68U == parse_c[0]);
//    CHECK(58U == parse_c[1]);
//    CHECK(1u == subsubcom->count());
//    CHECK(0u == subcom2->count());
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: DuplicateSubcommandCallbacks", "[config]")
//{
//
//    TempFile tmptoml{"TesttomlTmp.toml"};
//
//    app.set_config("--config", tmptoml);
//
//    {
//        std::ofstream out{tmptoml};
//        out << "[[foo]]" << std::endl;
//        out << "[[foo]]" << std::endl;
//        out << "[[foo]]" << std::endl;
//    }
//
//    auto* foo = app.add_subcommand("foo");
//    int count{0};
//    foo->callback([&count]() { ++count; });
//    foo->immediate_callback();
//    CHECK(foo->get_immediate_callback());
//    foo->configurable();
//
//    run();
//    CHECK(3 == count);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: SubcommandCallbackSingle", "[config]")
//{
//
//    TempFile tmptoml{"Testtomlcallback.toml"};
//
//    app.set_config("--config", tmptoml);
//
//    {
//        std::ofstream out{tmptoml};
//        out << "[foo]" << std::endl;
//    }
//    int count{0};
//    auto* foo = app.add_subcommand("foo");
//    foo->configurable();
//    foo->callback([&count]() { ++count; });
//
//    run();
//    CHECK(1 == count);
//}
//
//TEST_CASE_METHOD(TApp, "YamlFailure", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//    app.allow_config_extras(false);
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//    }
//
//    CHECK_THROWS_AS(run(), CLI::ConfigError);
//}
//
//TEST_CASE_METHOD(TApp, "YamlConfigurable", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//    bool value{false};
//    app.add_flag("--val", value)->configurable(true);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//    }
//
//    REQUIRE_NOTHROW(run());
//    CHECK(value);
//}
//
//TEST_CASE_METHOD(TApp, "YamlNotConfigurable", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//    bool value{false};
//    app.add_flag("--val", value)->configurable(false);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "val=1" << std::endl;
//    }
//
//    CHECK_THROWS_AS(run(), CLI::ConfigError);
//}
//
//TEST_CASE_METHOD(TApp, "YamlSubFailure", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.add_subcommand("other");
//    app.set_config("--config", tmpYaml);
//    app.allow_config_extras(false);
//    {
//        std::ofstream out{tmpYaml};
//        out << "[other]" << std::endl;
//        out << "val=1" << std::endl;
//    }
//
//    CHECK_THROWS_AS(run(), CLI::ConfigError);
//}
//
//TEST_CASE_METHOD(TApp, "YamlNoSubFailure", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//    app.allow_config_extras(CLI::config_extras_mode::error);
//    {
//        std::ofstream out{tmpYaml};
//        out << "[other]" << std::endl;
//        out << "val=1" << std::endl;
//    }
//
//    CHECK_THROWS_AS(run(), CLI::ConfigError);
//}
//
//TEST_CASE_METHOD(TApp, "YamlFlagConvertFailure", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.add_flag("--flag");
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "flag=moobook" << std::endl;
//    }
//    run();
//    bool result{false};
//    auto* opt = app.get_option("--flag");
//    CHECK_THROWS_AS(opt->results(result), CLI::ConversionError);
//    std::string res;
//    opt->results(res);
//    CHECK("moobook" == res);
//}
//
//TEST_CASE_METHOD(TApp, "YamlFlagNumbers", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    bool boo{false};
//    app.add_flag("--flag", boo);
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "flag=3" << std::endl;
//    }
//
//    REQUIRE_NOTHROW(run());
//    CHECK(boo);
//}
//
//TEST_CASE_METHOD(TApp, "YamlFlagDual", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    bool boo{false};
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    app.add_flag("--flag", boo);
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "flag=1 1" << std::endl;
//    }
//
//    CHECK_THROWS_AS(run(), CLI::ConversionError);
//}
//
//TEST_CASE_METHOD(TApp, "YamlShort", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    int key{0};
//    app.add_option("--flag,-f", key);
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "f=3" << std::endl;
//    }
//
//    REQUIRE_NOTHROW(run());
//    CHECK(3 == key);
//}
//
//TEST_CASE_METHOD(TApp, "YamlDefaultPath", "[config]")
//{
//
//    TempFile tmpYaml{"../TestIniTmp.ini"};
//
//    int key{0};
//    app.add_option("--flag,-f", key);
//    app.set_config("--config", "TestIniTmp.ini")->transform(CLI::FileOnDefaultPath("../"));
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "f=3" << std::endl;
//    }
//
//    REQUIRE_NOTHROW(run());
//    CHECK(3 == key);
//}
//
//TEST_CASE_METHOD(TApp, "YamlMultipleDefaultPath", "[config]")
//{
//
//    TempFile tmpYaml{"../TestIniTmp.ini"};
//
//    int key{0};
//    app.add_option("--flag,-f", key);
//    auto* cfgOption = app.set_config("--config", "doesnotexist.ini")
//            ->transform(CLI::FileOnDefaultPath("../"))
//            ->transform(CLI::FileOnDefaultPath("../other", false));
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "f=3" << std::endl;
//    }
//
//    args = {"--config", "TestIniTmp.ini"};
//    REQUIRE_NOTHROW(run());
//    CHECK(3 == key);
//    CHECK(cfgOption->as<std::string>() == "../TestIniTmp.ini");
//}
//
//TEST_CASE_METHOD(TApp, "YamlMultipleDefaultPathAlternate", "[config]")
//{
//
//    TempFile tmpYaml{"../TestIniTmp.ini"};
//
//    int key{0};
//    app.add_option("--flag,-f", key);
//    auto* cfgOption = app.set_config("--config", "doesnotexist.ini")
//            ->transform(CLI::FileOnDefaultPath("../other") | CLI::FileOnDefaultPath("../"));
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "f=3" << std::endl;
//    }
//
//    args = {"--config", "TestIniTmp.ini"};
//    REQUIRE_NOTHROW(run());
//    CHECK(3 == key);
//    CHECK(cfgOption->as<std::string>() == "../TestIniTmp.ini");
//}
//
//TEST_CASE_METHOD(TApp, "YamlPositional", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    int key{0};
//    app.add_option("key", key);
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "key=3" << std::endl;
//    }
//
//    REQUIRE_NOTHROW(run());
//    CHECK(3 == key);
//}
//
//TEST_CASE_METHOD(TApp, "YamlEnvironmental", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    int key{0};
//    app.add_option("key", key)->envname("CLI11_TEST_ENV_KEY_TMP");
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "CLI11_TEST_ENV_KEY_TMP=3" << std::endl;
//    }
//
//    REQUIRE_NOTHROW(run());
//    CHECK(3 == key);
//}
//
//TEST_CASE_METHOD(TApp, "YamlFlagText", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    bool flag1{false}, flag2{false}, flag3{false}, flag4{false};
//    app.add_flag("--flag1", flag1);
//    app.add_flag("--flag2", flag2);
//    app.add_flag("--flag3", flag3);
//    app.add_flag("--flag4", flag4);
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "flag1=true" << std::endl;
//        out << "flag2=on" << std::endl;
//        out << "flag3=off" << std::endl;
//        out << "flag4=1" << std::endl;
//    }
//
//    run();
//
//    CHECK(flag1);
//    CHECK(flag2);
//    CHECK(!flag3);
//    CHECK(flag4);
//}
//
//TEST_CASE_METHOD(TApp, "YamlFlags", "[config]")
//{
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=2" << std::endl;
//        out << "three=true" << std::endl;
//        out << "four=on" << std::endl;
//        out << "five" << std::endl;
//    }
//
//    int two{0};
//    bool three{false}, four{false}, five{false};
//    app.add_flag("--two", two);
//    app.add_flag("--three", three);
//    app.add_flag("--four", four);
//    app.add_flag("--five", five);
//
//    run();
//
//    CHECK(two == 2);
//    CHECK(three);
//    CHECK(four);
//    CHECK(five);
//}
//
//TEST_CASE_METHOD(TApp, "YamlFalseFlags", "[config]")
//{
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=-2" << std::endl;
//        out << "three=false" << std::endl;
//        out << "four=1" << std::endl;
//        out << "five" << std::endl;
//    }
//
//    int two{0};
//    bool three{false}, four{false}, five{false};
//    app.add_flag("--two", two);
//    app.add_flag("--three", three);
//    app.add_flag("--four", four);
//    app.add_flag("--five", five);
//
//    run();
//
//    CHECK(two == -2);
//    CHECK(!three);
//    CHECK(four);
//    CHECK(five);
//}
//
//TEST_CASE_METHOD(TApp, "YamlFalseFlagsDef", "[config]")
//{
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=2" << std::endl;
//        out << "three=true" << std::endl;
//        out << "four=on" << std::endl;
//        out << "five" << std::endl;
//    }
//
//    int two{0};
//    bool three{false}, four{false}, five{false};
//    app.add_flag("--two{false}", two);
//    app.add_flag("--three", three);
//    app.add_flag("!--four", four);
//    app.add_flag("--five", five);
//
//    run();
//
//    CHECK(two == -2);
//    CHECK(three);
//    CHECK(!four);
//    CHECK(five);
//}
//
//TEST_CASE_METHOD(TApp, "YamlFalseFlagsDefDisableOverrideError", "[config]")
//{
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=2" << std::endl;
//        out << "four=on" << std::endl;
//        out << "five" << std::endl;
//    }
//
//    int two{0};
//    bool four{false}, five{false};
//    app.add_flag("--two{false}", two)->disable_flag_override();
//    app.add_flag("!--four", four);
//    app.add_flag("--five", five);
//
//    CHECK_THROWS_AS(run(), CLI::ArgumentMismatch);
//}
//
//TEST_CASE_METHOD(TApp, "YamlFalseFlagsDefDisableOverrideSuccess", "[config]")
//{
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//    app.set_config("--config", tmpYaml);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "[default]" << std::endl;
//        out << "two=2" << std::endl;
//        out << "four={}" << std::endl;
//        out << "val=15" << std::endl;
//    }
//
//    int two{0}, four{0}, val{0};
//    app.add_flag("--two{2}", two)->disable_flag_override();
//    app.add_flag("--four{4}", four)->disable_flag_override();
//    app.add_flag("--val", val);
//
//    run();
//
//    CHECK(two == 2);
//    CHECK(four == 4);
//    CHECK(val == 15);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputSimple", "[config]")
//{
//
//    int v{0};
//    app.add_option("--simple", v);
//
//    args = {"--simple=3"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "simple=3\n");
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputShort", "[config]")
//{
//
//    int v{0};
//    app.add_option("-s", v);
//
//    args = {"-s3"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "s=3\n");
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputPositional", "[config]")
//{
//
//    int v{0};
//    app.add_option("pos", v);
//
//    args = {"3"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "pos=3\n");
//}
//
//// try the output with environmental only arguments
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputEnvironmental", "[config]")
//{
//
//    put_env("CLI11_TEST_ENV_TMP", "2");
//
//    int val{1};
//    app.add_option(std::string{}, val)->envname("CLI11_TEST_ENV_TMP");
//
//    run();
//
//    CHECK(val == 2);
//    std::string str = app.config_to_str();
//    CHECK(str == "CLI11_TEST_ENV_TMP=2\n");
//
//    unset_env("CLI11_TEST_ENV_TMP");
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputNoConfigurable", "[config]")
//{
//
//    int v1{0}, v2{0};
//    app.add_option("--simple", v1);
//    app.add_option("--noconf", v2)->configurable(false);
//
//    args = {"--simple=3", "--noconf=2"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "simple=3\n");
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputShortSingleDescription", "[config]")
//{
//    std::string flag = "some_flag";
//    const std::string description = "Some short description.";
//    app.add_flag("--" + flag, description);
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("# " + description + "\n" + flag + "=false\n"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputShortDoubleDescription", "[config]")
//{
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1);
//    app.add_flag("--" + flag2, description2);
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    std::string ans = "# " + description1 + "\n" + flag1 + "=false\n\n# " + description2 + "\n" + flag2 + "=false\n";
//    CHECK_THAT(str, ContainsSubstring(ans));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputGroups", "[config]")
//{
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("group1"));
//    CHECK_THAT(str, ContainsSubstring("group2"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputHiddenOptions", "[config]")
//{
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    double val{12.7};
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//    app.add_option("--dval", val)->capture_default_str()->group("");
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("group1"));
//    CHECK_THAT(str, ContainsSubstring("group2"));
//    CHECK_THAT(str, ContainsSubstring("dval=12.7"));
//    auto loc = str.find("dval=12.7");
//    auto locg1 = str.find("group1");
//    CHECK(loc < locg1);
//    // make sure it doesn't come twice
//    loc = str.find("dval=12.7", loc + 4);
//    CHECK(std::string::npos == loc);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputAppMultiLineDescription", "[config]")
//{
//    app.description("Some short app description.\n"
//                    "That has multiple lines.");
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("# Some short app description.\n"));
//    CHECK_THAT(str, ContainsSubstring("# That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputMultiLineDescription", "[config]")
//{
//    std::string flag = "some_flag";
//    const std::string description = "Some short description.\nThat has lines.";
//    app.add_flag("--" + flag, description);
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("# Some short description.\n"));
//    CHECK_THAT(str, ContainsSubstring("# That has lines.\n"));
//    CHECK_THAT(str, ContainsSubstring(flag + "=false\n"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputOptionGroupMultiLineDescription", "[config]")
//{
//    std::string flag = "flag";
//    const std::string description = "Short flag description.\n";
//    auto* og = app.add_option_group("group");
//    og->description("Option group description.\n"
//                    "That has multiple lines.");
//    og->add_flag("--" + flag, description);
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("# Option group description.\n"));
//    CHECK_THAT(str, ContainsSubstring("# That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputSubcommandMultiLineDescription", "[config]")
//{
//    std::string flag = "flag";
//    const std::string description = "Short flag description.\n";
//    auto* subcom = app.add_subcommand("subcommand");
//    subcom->configurable();
//    subcom->description("Subcommand description.\n"
//                        "That has multiple lines.");
//    subcom->add_flag("--" + flag, description);
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("# Subcommand description.\n"));
//    CHECK_THAT(str, ContainsSubstring("# That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputOptionGroup", "[config]")
//{
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    double val{12.7};
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//    auto* og = app.add_option_group("group3", "g3 desc");
//    og->add_option("--dval", val)->capture_default_str()->group("");
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("group1"));
//    CHECK_THAT(str, ContainsSubstring("group2"));
//    CHECK_THAT(str, ContainsSubstring("dval=12.7"));
//    CHECK_THAT(str, ContainsSubstring("group3"));
//    CHECK_THAT(str, ContainsSubstring("g3 desc"));
//    auto loc = str.find("dval=12.7");
//    auto locg1 = str.find("group1");
//    auto locg3 = str.find("group3");
//    CHECK(loc > locg1);
//    // make sure it doesn't come twice
//    loc = str.find("dval=12.7", loc + 4);
//    CHECK(std::string::npos == loc);
//    CHECK(locg1 < locg3);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputVector", "[config]")
//{
//
//    std::vector<int> v;
//    app.add_option("--vector", v);
//    app.config_formatter(std::make_shared<CLI::ConfigTOML>());
//    args = {"--vector", "1", "2", "3"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "vector=[1, 2, 3]\n");
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputTuple", "[config]")
//{
//
//    std::tuple<double, double, double, double> t;
//    app.add_option("--tuple", t);
//    app.config_formatter(std::make_shared<CLI::ConfigTOML>());
//    args = {"--tuple", "1", "2", "3", "4"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "tuple=[1, 2, 3, 4]\n");
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: ConfigOutputVectorCustom", "[config]")
//{
//
//    std::vector<int> v;
//    app.add_option("--vector", v);
//    auto V = std::make_shared<CLI::ConfigBase>();
//    V->arrayBounds('{', '}')->arrayDelimiter(';')->valueSeparator(':');
//    app.config_formatter(V);
//    args = {"--vector", "1", "2", "3"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "vector:{1; 2; 3}\n");
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputFlag", "[config]")
//{
//
//    int v{0}, q{0};
//    app.add_option("--simple", v);
//    app.add_flag("--nothing");
//    app.add_flag("--onething");
//    app.add_flag("--something", q);
//
//    args = {"--simple=3", "--onething", "--something", "--something"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=3"));
//    CHECK_THAT(str, !ContainsSubstring("nothing"));
//    CHECK_THAT(str, ContainsSubstring("onething=true"));
//    CHECK_THAT(str, ContainsSubstring("something=2"));
//
//    str = app.config_to_str(true);
//    CHECK_THAT(str, ContainsSubstring("nothing"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputSet", "[config]")
//{
//
//    int v{0};
//    app.add_option("--simple", v)->check(CLI::IsMember({1, 2, 3}));
//
//    args = {"--simple=2"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=2"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputDefault", "[config]")
//{
//
//    int v{7};
//    app.add_option("--simple", v)->capture_default_str();
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, !ContainsSubstring("simple=7"));
//
//    str = app.config_to_str(true);
//    CHECK_THAT(str, ContainsSubstring("simple=7"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputSubcom", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other");
//    subcom->add_flag("--newer");
//
//    args = {"--simple", "other", "--newer"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("other.newer=true"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputSubcomConfigurable", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other")->configurable();
//    subcom->add_flag("--newer");
//
//    args = {"--simple", "other", "--newer"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("[other]"));
//    CHECK_THAT(str, ContainsSubstring("newer=true"));
//    CHECK(std::string::npos == str.find("other.newer=true"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputSubsubcom", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other");
//    subcom->add_flag("--newer");
//    auto* subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//
//    args = {"--simple", "other", "--newer", "sub2", "--newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("other.newer=true"));
//    CHECK_THAT(str, ContainsSubstring("other.sub2.newest=true"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputSubsubcomConfigurable", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other")->configurable();
//    subcom->add_flag("--newer");
//
//    auto* subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//
//    args = {"--simple", "other", "--newer", "sub2", "--newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("[other]"));
//    CHECK_THAT(str, ContainsSubstring("newer=true"));
//    CHECK_THAT(str, ContainsSubstring("[other.sub2]"));
//    CHECK_THAT(str, ContainsSubstring("newest=true"));
//    CHECK(std::string::npos == str.find("sub2.newest=true"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputSubcomNonConfigurable", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other", "other_descriptor")->configurable();
//    subcom->add_flag("--newer");
//
//    auto* subcom2 = app.add_subcommand("sub2", "descriptor2");
//    subcom2->add_flag("--newest")->configurable(false);
//
//    args = {"--simple", "other", "--newer", "sub2", "--newest"};
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("other_descriptor"));
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("[other]"));
//    CHECK_THAT(str, ContainsSubstring("newer=true"));
//    CHECK_THAT(str, !ContainsSubstring("newest"));
//    CHECK_THAT(str, !ContainsSubstring("descriptor2"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputSubsubcomConfigurableDeep", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other")->configurable();
//    subcom->add_flag("--newer");
//
//    auto* subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//    auto* sssscom = subsubcom->add_subcommand("sub-level2");
//    subsubcom->add_flag("--still_newer");
//    auto* s5com = sssscom->add_subcommand("sub-level3");
//    s5com->add_flag("--absolute_newest");
//
//    args = {"--simple", "other", "sub2", "sub-level2", "sub-level3", "--absolute_newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("[other.sub2.sub-level2.sub-level3]"));
//    CHECK_THAT(str, ContainsSubstring("absolute_newest=true"));
//    CHECK(std::string::npos == str.find(".absolute_newest=true"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: TomlOutputQuoted", "[config]")
//{
//
//    std::string val1;
//    app.add_option("--val1", val1);
//
//    std::string val2;
//    app.add_option("--val2", val2);
//
//    args = {"--val1", "I am a string", "--val2", R"(I am a "confusing" string)"};
//
//    run();
//
//    CHECK(val1 == "I am a string");
//    CHECK(val2 == "I am a \"confusing\" string");
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("val1=\"I am a string\""));
//    CHECK_THAT(str, ContainsSubstring("val2='I am a \"confusing\" string'"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: DefaultsTomlOutputQuoted", "[config]")
//{
//
//    std::string val1{"I am a string"};
//    app.add_option("--val1", val1)->capture_default_str();
//
//    std::string val2{R"(I am a "confusing" string)"};
//    app.add_option("--val2", val2)->capture_default_str();
//
//    run();
//
//    std::string str = app.config_to_str(true);
//    CHECK_THAT(str, ContainsSubstring("val1=\"I am a string\""));
//    CHECK_THAT(str, ContainsSubstring("val2='I am a \"confusing\" string'"));
//}
//
//// #298
//TEST_CASE_METHOD(TApp, "Yaml: StopReadingConfigOnClear", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.set_config("--config", tmpYaml);
//    auto* ptr = app.set_config();  // Should *not* read config file
//    CHECK(nullptr == ptr);
//
//    {
//        std::ofstream out{tmpYaml};
//        out << "volume=1" << std::endl;
//    }
//
//    int volume{0};
//    app.add_option("--volume", volume, "volume1");
//
//    run();
//
//    CHECK(0 == volume);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: ConfigWriteReadWrite", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//
//    app.add_flag("--flag");
//    run();
//
//    // Save config, with default values too
//    std::string config1 = app.config_to_str(true, true);
//    {
//        std::ofstream out{tmpYaml};
//        out << config1 << std::endl;
//    }
//
//    app.set_config("--config", tmpYaml, "Read an ini file", true);
//    run();
//
//    std::string config2 = app.config_to_str(true, true);
//
//    CHECK(config2 == config1);
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: ConfigWriteReadNegated", "[config]")
//{
//
//    TempFile tmpYaml{"TestYamlTmp.yaml"};
//    bool flag{true};
//    app.add_flag("!--no-flag", flag);
//    args = {"--no-flag"};
//    run();
//
//    // Save config, with default values too
//    std::string config1 = app.config_to_str(false, false);
//    {
//        std::ofstream out{tmpYaml};
//        out << config1 << std::endl;
//    }
//    CHECK_FALSE(flag);
//    args.clear();
//    flag = true;
//    app.set_config("--config", tmpYaml, "Read an ini file", true);
//    run();
//
//    CHECK_FALSE(flag);
//}
//
///////// INI output tests
//
//TEST_CASE_METHOD(TApp, "YamlOutputSimple", "[config]")
//{
//
//    int v{0};
//    app.add_option("--simple", v);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    args = {"--simple=3"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "simple=3\n");
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputNoConfigurable", "[config]")
//{
//
//    int v1{0}, v2{0};
//    app.add_option("--simple", v1);
//    app.add_option("--noconf", v2)->configurable(false);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    args = {"--simple=3", "--noconf=2"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "simple=3\n");
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputShortSingleDescription", "[config]")
//{
//    std::string flag = "some_flag";
//    const std::string description = "Some short description.";
//    app.add_flag("--" + flag, description);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("; " + description + "\n" + flag + "=false\n"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputShortDoubleDescription", "[config]")
//{
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1);
//    app.add_flag("--" + flag2, description2);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    std::string ans = "; " + description1 + "\n" + flag1 + "=false\n\n; " + description2 + "\n" + flag2 + "=false\n";
//    CHECK_THAT(str, ContainsSubstring(ans));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputGroups", "[config]")
//{
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("group1"));
//    CHECK_THAT(str, ContainsSubstring("group2"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputHiddenOptions", "[config]")
//{
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    double val{12.7};
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//    app.add_option("--dval", val)->capture_default_str()->group("");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("group1"));
//    CHECK_THAT(str, ContainsSubstring("group2"));
//    CHECK_THAT(str, ContainsSubstring("dval=12.7"));
//    auto loc = str.find("dval=12.7");
//    auto locg1 = str.find("group1");
//    CHECK(loc < locg1);
//    // make sure it doesn't come twice
//    loc = str.find("dval=12.7", loc + 4);
//    CHECK(std::string::npos == loc);
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputAppMultiLineDescription", "[config]")
//{
//    app.description("Some short app description.\n"
//                    "That has multiple lines.");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("; Some short app description.\n"));
//    CHECK_THAT(str, ContainsSubstring("; That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputMultiLineDescription", "[config]")
//{
//    std::string flag = "some_flag";
//    const std::string description = "Some short description.\nThat has lines.";
//    app.add_flag("--" + flag, description);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("; Some short description.\n"));
//    CHECK_THAT(str, ContainsSubstring("; That has lines.\n"));
//    CHECK_THAT(str, ContainsSubstring(flag + "=false\n"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputOptionGroupMultiLineDescription", "[config]")
//{
//    std::string flag = "flag";
//    const std::string description = "Short flag description.\n";
//    auto* og = app.add_option_group("group");
//    og->description("Option group description.\n"
//                    "That has multiple lines.");
//    og->add_flag("--" + flag, description);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("; Option group description.\n"));
//    CHECK_THAT(str, ContainsSubstring("; That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputSubcommandMultiLineDescription", "[config]")
//{
//    std::string flag = "flag";
//    const std::string description = "Short flag description.\n";
//    auto* subcom = app.add_subcommand("subcommand");
//    subcom->configurable();
//    subcom->description("Subcommand description.\n"
//                        "That has multiple lines.");
//    subcom->add_flag("--" + flag, description);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("; Subcommand description.\n"));
//    CHECK_THAT(str, ContainsSubstring("; That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputOptionGroup", "[config]")
//{
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    double val{12.7};
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//    auto* og = app.add_option_group("group3", "g3 desc");
//    og->add_option("--dval", val)->capture_default_str()->group("");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, ContainsSubstring("group1"));
//    CHECK_THAT(str, ContainsSubstring("group2"));
//    CHECK_THAT(str, ContainsSubstring("dval=12.7"));
//    CHECK_THAT(str, ContainsSubstring("group3"));
//    CHECK_THAT(str, ContainsSubstring("g3 desc"));
//    auto loc = str.find("dval=12.7");
//    auto locg1 = str.find("group1");
//    auto locg3 = str.find("group3");
//    CHECK(loc > locg1);
//    // make sure it doesn't come twice
//    loc = str.find("dval=12.7", loc + 4);
//    CHECK(std::string::npos == loc);
//    CHECK(locg1 < locg3);
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputVector", "[config]")
//{
//
//    std::vector<int> v;
//    app.add_option("--vector", v);
//
//    args = {"--vector", "1", "2", "3"};
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "vector=1 2 3\n");
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputFlag", "[config]")
//{
//
//    int v{0}, q{0};
//    app.add_option("--simple", v);
//    app.add_flag("--nothing");
//    app.add_flag("--onething");
//    app.add_flag("--something", q);
//
//    args = {"--simple=3", "--onething", "--something", "--something"};
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=3"));
//    CHECK_THAT(str, !ContainsSubstring("nothing"));
//    CHECK_THAT(str, ContainsSubstring("onething=true"));
//    CHECK_THAT(str, ContainsSubstring("something=2"));
//
//    str = app.config_to_str(true);
//    CHECK_THAT(str, ContainsSubstring("nothing"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputSet", "[config]")
//{
//
//    int v{0};
//    app.add_option("--simple", v)->check(CLI::IsMember({1, 2, 3}));
//
//    args = {"--simple=2"};
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=2"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputDefault", "[config]")
//{
//
//    int v{7};
//    app.add_option("--simple", v)->capture_default_str();
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, !ContainsSubstring("simple=7"));
//
//    str = app.config_to_str(true);
//    CHECK_THAT(str, ContainsSubstring("simple=7"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputSubcom", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other");
//    subcom->add_flag("--newer");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    args = {"--simple", "other", "--newer"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("other.newer=true"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputSubcomCustomSep", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other");
//    subcom->add_flag("--newer");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    app.get_config_formatter_base()->parentSeparator(':');
//    args = {"--simple", "other", "--newer"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("other:newer=true"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputSubcomConfigurable", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other")->configurable();
//    subcom->add_flag("--newer");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    args = {"--simple", "other", "--newer"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("[other]"));
//    CHECK_THAT(str, ContainsSubstring("newer=true"));
//    CHECK(std::string::npos == str.find("other.newer=true"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputSubsubcom", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other");
//    subcom->add_flag("--newer");
//    auto* subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    args = {"--simple", "other", "--newer", "sub2", "--newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("other.newer=true"));
//    CHECK_THAT(str, ContainsSubstring("other.sub2.newest=true"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputSubsubcomCustomSep", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other");
//    subcom->add_flag("--newer");
//    auto* subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    app.get_config_formatter_base()->parentSeparator('|');
//    args = {"--simple", "other", "--newer", "sub2", "--newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("other|newer=true"));
//    CHECK_THAT(str, ContainsSubstring("other|sub2|newest=true"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputSubsubcomConfigurable", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other")->configurable();
//    subcom->add_flag("--newer");
//
//    auto* subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    args = {"--simple", "other", "--newer", "sub2", "--newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("[other]"));
//    CHECK_THAT(str, ContainsSubstring("newer=true"));
//    CHECK_THAT(str, ContainsSubstring("[other.sub2]"));
//    CHECK_THAT(str, ContainsSubstring("newest=true"));
//    CHECK(std::string::npos == str.find("sub2.newest=true"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputSubsubcomConfigurableDeep", "[config]")
//{
//
//    app.add_flag("--simple");
//    auto* subcom = app.add_subcommand("other")->configurable();
//    subcom->add_flag("--newer");
//
//    auto* subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//    auto* sssscom = subsubcom->add_subcommand("sub-level2");
//    subsubcom->add_flag("--still_newer");
//    auto* s5com = sssscom->add_subcommand("sub-level3");
//    s5com->add_flag("--absolute_newest");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    args = {"--simple", "other", "sub2", "sub-level2", "sub-level3", "--absolute_newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("simple=true"));
//    CHECK_THAT(str, ContainsSubstring("[other.sub2.sub-level2.sub-level3]"));
//    CHECK_THAT(str, ContainsSubstring("absolute_newest=true"));
//    CHECK(std::string::npos == str.find(".absolute_newest=true"));
//}
//
//TEST_CASE_METHOD(TApp, "YamlOutputQuoted", "[config]")
//{
//
//    std::string val1;
//    app.add_option("--val1", val1);
//
//    std::string val2;
//    app.add_option("--val2", val2);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    args = {"--val1", "I am a string", "--val2", R"(I am a "confusing" string)"};
//
//    run();
//
//    CHECK(val1 == "I am a string");
//    CHECK(val2 == "I am a \"confusing\" string");
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, ContainsSubstring("val1=\"I am a string\""));
//    CHECK_THAT(str, ContainsSubstring("val2='I am a \"confusing\" string'"));
//}
//
//TEST_CASE_METHOD(TApp, "Yaml: DefaultsIniOutputQuoted", "[config]")
//{
//
//    std::string val1{"I am a string"};
//    app.add_option("--val1", val1)->capture_default_str();
//
//    std::string val2{R"(I am a "confusing" string)"};
//    app.add_option("--val2", val2)->capture_default_str();
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true);
//    CHECK_THAT(str, ContainsSubstring("val1=\"I am a string\""));
//    CHECK_THAT(str, ContainsSubstring("val2='I am a \"confusing\" string'"));
//}
