#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

namespace perfsight {
namespace plugins {

// Metric data structure
struct MetricValue {
    std::string name;
    std::string value;
    std::string unit;
    std::string description;
    
    MetricValue(const std::string& n = "", const std::string& v = "", 
                const std::string& u = "", const std::string& d = "")
        : name(n), value(v), unit(u), description(d) {}
};

using MetricData = std::vector<MetricValue>;

// Plugin parameters
using PluginParams = std::map<std::string, std::string>;

/**
 * @brief Interface for all metric plugins
 * 
 * Each plugin is responsible for collecting specific metrics from the system.
 * Plugins must be thread-safe and handle failures gracefully.
 */
class IMetricPlugin {
public:
    virtual ~IMetricPlugin() = default;

    /**
     * @brief Initialize the plugin with parameters
     * @param params Configuration parameters from YAML
     * @return true if initialization successful
     */
    virtual bool initialize(const PluginParams& params) = 0;

    /**
     * @brief Collect metrics
     * @return Vector of collected metrics
     */
    virtual MetricData collectMetrics() = 0;

    /**
     * @brief Get plugin name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Cleanup resources
     */
    virtual void shutdown() = 0;

    /**
     * @brief Check if plugin is healthy
     */
    virtual bool isHealthy() const = 0;
};

using PluginPtr = std::shared_ptr<IMetricPlugin>;

} // namespace plugins
} // namespace perfsight
