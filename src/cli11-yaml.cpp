#include <cli11-yaml/cli11-yaml.hpp>

#include <yaml-cpp/yaml.h>

namespace CLI {

std::vector<ConfigItem>
ConfigYAML::from_config(std::istream& is) const
{
    YAML::Node node = YAML::Load(is);

    std::vector<ConfigItem> output;
    std::vector<std::string> parents;

//    std::cout << "Create Parent: " << detail::join(parents) << std::endl;
    switch (node.Type()) {
        case YAML::NodeType::Null: {
//        std::cout << indent(level) << "Null" << std::endl;

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
//        std::cout << indent(level) << "Scalar: " << node.as<std::string>() << std::endl;
            break;
        }
        case YAML::NodeType::Sequence: {
//        std::cout << indent(level) << "Sequence: " << std::endl;

            ConfigItem config_item;
            config_item.name = parents.empty() ? "" : *parents.rbegin();

            config_item.parents = parents;
            if (!config_item.parents.empty())
                config_item.parents.erase(config_item.parents.rbegin().base());

            for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
                if (it->IsScalar()) {
//                std::cout << indent(level + 1) << "Value: " << it->as<std::string>() << '\n';

                    config_item.inputs.push_back(it->as<std::string>());
                }
                else {
//                    for (auto ci: parse(*it, parents, level + 1)) {
//                        output.push_back(std::move(ci));
//                    }
                }
            }
            output.push_back(std::move(config_item));
            break;
        }
        case YAML::NodeType::Map: {
//        std::cout << indent(level) << "Map: " << std::endl;

            for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
//            std::cout << indent(level) << "Key: " << it->first.as<std::string>();
                if (it->second.IsScalar()) {
//                std::cout << ": Value: " << it->second.as<std::string>() << '\n';

                    ConfigItem config_item;
                    config_item.name = it->first.as<std::string>();
                    config_item.parents = parents;
                    config_item.inputs.push_back(it->second.as<std::string>());
                    output.push_back(std::move(config_item));

                }
                else {
//                std::cout << ":\n";
                    auto tmp = parents;
                    tmp.push_back(it->first.as<std::string>());
//                    for (auto ci: parse(it->second, tmp, level + 1)) {
//                        output.push_back(std::move(ci));
//                    }
                }
            }
            break;
        }
        case YAML::NodeType::Undefined:
//        std::cout << indent(level) << "Undefined" << std::endl;
            break;
        default:
//        std::cout << indent(level) << "Unknown Type" << std::endl;
            break;
    }

    return output;
}

}
