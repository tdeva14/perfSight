#pragma once

#include "IMetricPlugin.hpp"
#include <set>

namespace perfsight {
namespace plugins {

/**
 * @brief Plugin for collecting /proc-based system metrics
 * 
 * Collects:
 * - System memory (from /proc/meminfo)
 * - Process memory (from /proc/<pid>/status)
 * - CPU usage (from /proc/stat)
 * - Swap usage
 */
class ProcMetricsPlugin : public IMetricPlugin {
public:
    ProcMetricsPlugin(const std::string& metricName);
    ~ProcMetricsPlugin() override = default;

    bool initialize(const PluginParams& params) override;
    MetricData collectMetrics() override;
    std::string getName() const override { return metricName_; }
    void shutdown() override;
    bool isHealthy() const override { return healthy_; }

private:
    std::string metricName_;
    bool healthy_;
    
    // Configuration
    std::set<std::string> processWhitelist_;
    bool includeKernelThreads_;
    
    // Metric collection methods
    MetricData collectSystemMemory();
    MetricData collectProcessMemory();
    MetricData collectCPUUsage();
    
    // Helper methods
    std::string readFile(const std::string& path);
    std::vector<std::string> getProcessList();
    bool isProcessInWhitelist(const std::string& processName);
};

} // namespace plugins
} // namespace perfsight
