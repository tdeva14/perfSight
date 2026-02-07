#include "core/MetricsAggregator.hpp"
#include "core/Logger.hpp"
#include <thread>
#include <future>
#include <algorithm>

namespace perfsight {
namespace core {

MetricsAggregator::MetricsAggregator(int maxParallelPlugins)
    : maxParallelPlugins_(maxParallelPlugins) {
}

void MetricsAggregator::addPlugin(plugins::PluginPtr plugin) {
    if (plugin) {
        plugins_.push_back(plugin);
    }
}

std::map<std::string, plugins::MetricData> MetricsAggregator::collectAllMetrics() {
    auto& logger = Logger::getInstance();
    std::map<std::string, plugins::MetricData> allMetrics;
    
    logger.debug("Starting metric collection from ", plugins_.size(), " plugins");
    
    // Simple sequential collection (can be enhanced with parallel execution)
    for (auto& plugin : plugins_) {
        try {
            if (plugin->isHealthy()) {
                auto metrics = plugin->collectMetrics();
                allMetrics[plugin->getName()] = metrics;
                logger.debug("Collected ", metrics.size(), " metrics from ", plugin->getName());
            } else {
                logger.warning("Plugin ", plugin->getName(), " is not healthy, skipping");
            }
        } catch (const std::exception& e) {
            logger.error("Exception collecting metrics from ", plugin->getName(), ": ", e.what());
        }
    }
    
    logger.info("Metric collection complete. Total plugins: ", allMetrics.size());
    return allMetrics;
}

} // namespace core
} // namespace perfsight
