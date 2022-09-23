#pragma once

#include <CLI/Config.hpp>

namespace CLI {

// --------------------------------------------------------------------------
class ConfigYAML : public Config {
     public:
    /// Convert an app into a configuration
    std::string to_config(const App *, bool, bool, std::string) const override
    {
        return {};
    }

    /// Convert a configuration into an app
    std::vector<ConfigItem> from_config(std::istream& is) const override
    {
        return {};
    }

  private:
};

}
