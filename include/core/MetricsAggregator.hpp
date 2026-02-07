#pragma once

#include "plugins/IMetricPlugin.hpp"
#include <vector>
#include <memory>
#include <map>

namespace perfsight {
namespace core {

/**
 * @brief Aggregates metrics from multiple plugins
 */
class MetricsAggregator {
public:
    MetricsAggregator(int maxParallelPlugins);
    
    void addPlugin(plugins::PluginPtr plugin);
    
    /**
     * @brief Collect metrics from all plugins
     * @return Map of plugin name to collected metrics
     */
    std::map<std::string, plugins::MetricData> collectAllMetrics();

private:
    std::vector<plugins::PluginPtr> plugins_;
    int maxParallelPlugins_;
};

} // namespace core
} // namespace perfsight
