#pragma once

#include "IMetricPlugin.hpp"

namespace perfsight {
namespace plugins {

/**
 * @brief Plugin for collecting SoC-specific metrics
 * 
 * Collects:
 * - GPU memory usage
 * - CMA (Contiguous Memory Allocator) regions
 * - Memory bandwidth (SoC-specific)
 * 
 * Supports different SoC types: amlogic, broadcom, realtek
 */
class SoCMetricsPlugin : public IMetricPlugin {
public:
    SoCMetricsPlugin(const std::string& metricName);
    ~SoCMetricsPlugin() override = default;

    bool initialize(const PluginParams& params) override;
    MetricData collectMetrics() override;
    std::string getName() const override { return metricName_; }
    void shutdown() override;
    bool isHealthy() const override { return healthy_; }

private:
    std::string metricName_;
    std::string socType_;
    bool healthy_;
    
    // Metric collection methods
    MetricData collectGPUMemory();
    MetricData collectCMARegions();
    MetricData collectMemoryBandwidth();
    
    // Helper methods
    std::string readFile(const std::string& path);
    std::string getSoCSpecificPath(const std::string& metric);
};

} // namespace plugins
} // namespace perfsight
