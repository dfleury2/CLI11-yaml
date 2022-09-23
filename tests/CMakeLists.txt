add_executable(cli11yaml-test
    app_helper.hpp
    ConfigFileTest.cpp
    ConfigYamlTest.cpp
)

target_link_libraries(cli11yaml-test
    PUBLIC
        cli11-yaml::cli11-yaml
        Catch2::Catch2WithMain
)
