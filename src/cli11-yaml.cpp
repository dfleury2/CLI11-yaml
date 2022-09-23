#include <cli11-yaml/cli11-yaml.hpp>

namespace CLI {

std::string
ConfigYAML::to_config(const App*, bool, bool, std::string) const
{
    return {};
}

std::vector<ConfigItem>
ConfigYAML::from_config(std::istream& is) const
{
    YAML::Node config = YAML::Load(is);
    return parse(config, {});
}

std::vector<ConfigItem>
ConfigYAML::parse(const YAML::Node& node, std::vector<std::string> parents) const
{
    std::vector<ConfigItem> output;

    switch (node.Type()) {
        case YAML::NodeType::Null: {

            ConfigItem config_item;
            config_item.name = (parents.empty() ? "" : *parents.rbegin());
            config_item.parents = parents;
            if (!config_item.parents.empty())
                config_item.parents.erase(config_item.parents.rbegin().base());
            config_item.inputs.emplace_back("true");
            output.push_back(std::move(config_item));

            break;
        }
        case YAML::NodeType::Scalar: {
            break;
        }
        case YAML::NodeType::Sequence: {
            ConfigItem config_item;
            config_item.name = parents.empty() ? "" : *parents.rbegin();

            config_item.parents = parents;
            if (!config_item.parents.empty())
                config_item.parents.erase(config_item.parents.rbegin().base());

            for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
                if (it->IsScalar()) {
                    config_item.inputs.push_back(it->as<std::string>());
                }
                else {
                    for (auto ci: parse(*it, parents)) {
                        output.push_back(std::move(ci));
                    }
                }
            }
            output.push_back(std::move(config_item));
            break;
        }
        case YAML::NodeType::Map: {

            for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
                if (it->second.IsScalar()) {

                    ConfigItem config_item;
                    config_item.name = it->first.as<std::string>();
                    config_item.parents = parents;
                    config_item.inputs.push_back(it->second.as<std::string>());
                    output.push_back(std::move(config_item));

                }
                else {
                    auto tmp = parents;
                    tmp.push_back(it->first.as<std::string>());

                    if (it->second.IsMap()) { // Only Map start a section (not a sequence)
                        ConfigItem config_item_open;
                        config_item_open.name = "++";
                        config_item_open.parents = tmp;
                        output.push_back(std::move(config_item_open));
                    }

                    for (auto ci: parse(it->second, tmp)) {
                        output.push_back(std::move(ci));
                    }

                    if (it->second.IsMap()) { // Only Map end a section (not a sequence)
                        ConfigItem config_item_close;
                        config_item_close.name = "--";
                        config_item_close.parents = tmp;
                        output.push_back(std::move(config_item_close));
                    }
                }
            }
            break;
        }
        case YAML::NodeType::Undefined:
            break;
        default:
            break;
    }

    return output;
}

}
