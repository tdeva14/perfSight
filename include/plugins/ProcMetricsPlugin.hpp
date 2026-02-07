#pragma once

#include "IMetricPlugin.hpp"
#include <set>

namespace perfsight {
namespace plugins {

/**
 * @brief Plugin for collecting per-process metrics from /proc filesystem
 * 
 * Collects:
 * - Process stats (from /proc/[pid]/stat)
 * - Process memory maps (from /proc/[pid]/smaps)
 * - Process status (from /proc/[pid]/status)
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
    MetricData collectProcessMetrics();
    
    // Helper methods
    std::string readFile(const std::string& path);
    std::vector<std::string> getProcessList();
    bool isProcessInWhitelist(const std::string& processName);
    void parseProcessStat(const std::string& pid, const std::string& processName, MetricData& metrics);
    void parseProcessStatus(const std::string& pid, const std::string& processName, MetricData& metrics);
    void parseProcessSmaps(const std::string& pid, const std::string& processName, MetricData& metrics);
};

} // namespace plugins
} // namespace perfsight
