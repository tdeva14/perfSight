#include "core/PluginLoader.hpp"
#include "core/Logger.hpp"
#include "plugins/SystemMetricsPlugin.hpp"
#include "plugins/ProcMetricsPlugin.hpp"
#include "plugins/ContainerMetricsPlugin.hpp"

namespace perfsight {
namespace core {

plugins::PluginPtr PluginLoader::createPlugin(const MetricConfig& config) {
    auto& logger = Logger::getInstance();
    
    plugins::PluginPtr plugin;
    
    if (config.plugin == "system") {
        plugin = std::make_shared<plugins::SystemMetricsPlugin>(config.name);
    } else if (config.plugin == "proc") {
        plugin = std::make_shared<plugins::ProcMetricsPlugin>(config.name);
    } else if (config.plugin == "container") {
        plugin = std::make_shared<plugins::ContainerMetricsPlugin>(config.name);
    } else {
        logger.error("Unknown plugin type: ", config.plugin);
        return nullptr;
    }
    
    if (plugin && !plugin->initialize(config.params)) {
        logger.error("Failed to initialize plugin: ", config.name);
        return nullptr;
    }
    
    logger.info("Plugin created and initialized: ", config.name, " (", config.plugin, ")");
    return plugin;
}

} // namespace core
} // namespace perfsight
