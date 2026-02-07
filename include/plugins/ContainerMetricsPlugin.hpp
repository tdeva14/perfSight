#pragma once

#include "IMetricPlugin.hpp"

namespace perfsight {
namespace plugins {

/**
 * @brief Plugin for collecting container metrics
 * 
 * Collects memory and CPU usage from containers (Docker, LXC, etc.)
 * by reading cgroup information
 */
class ContainerMetricsPlugin : public IMetricPlugin {
public:
    ContainerMetricsPlugin(const std::string& metricName);
    ~ContainerMetricsPlugin() override = default;

    bool initialize(const PluginParams& params) override;
    MetricData collectMetrics() override;
    std::string getName() const override { return metricName_; }
    void shutdown() override;
    bool isHealthy() const override { return healthy_; }

private:
    std::string metricName_;
    bool healthy_;
    
    // Metric collection methods
    MetricData collectContainerMemory();
    
    // Helper methods
    std::string readFile(const std::string& path);
    std::vector<std::string> getContainerList();
};

} // namespace plugins
} // namespace perfsight
