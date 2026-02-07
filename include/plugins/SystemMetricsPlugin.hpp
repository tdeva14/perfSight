#pragma once

#include "IMetricPlugin.hpp"

namespace perfsight {
namespace plugins {

/**
 * @brief Plugin for collecting system-wide metrics from /proc filesystem
 * 
 * Collects:
 * - System memory (from /proc/meminfo)
 * - CPU usage (from /proc/stat)
 * - Memory fragmentation (from /proc/buddyinfo)
 */
class SystemMetricsPlugin : public IMetricPlugin {
public:
    SystemMetricsPlugin(const std::string& metricName);
    ~SystemMetricsPlugin() override = default;

    bool initialize(const PluginParams& params) override;
    MetricData collectMetrics() override;
    std::string getName() const override { return metricName_; }
    void shutdown() override;
    bool isHealthy() const override { return healthy_; }

private:
    std::string metricName_;
    bool healthy_;
    
    // Metric collection methods
    MetricData collectSystemMemory();
    MetricData collectCPUUsage();
    MetricData collectBuddyInfo();
    
    // Helper methods
    std::string readFile(const std::string& path);
};

} // namespace plugins
} // namespace perfsight
