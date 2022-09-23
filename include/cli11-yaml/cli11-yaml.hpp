#pragma once

#include <CLI/Config.hpp>
#include <yaml-cpp/yaml.h>

namespace CLI {

// --------------------------------------------------------------------------
class ConfigYAML : public Config {
public:
    /// Convert an app into a configuration
    std::string to_config(const App *, bool, bool, std::string) const override;

    /// Convert a configuration into an app
    std::vector<ConfigItem> from_config(std::istream& is) const override;

private:
    std::vector<ConfigItem> parse(const YAML::Node& node, std::vector<std::string> parents) const;
};

}
