#pragma once

#include "../plugins/IMetricPlugin.hpp"
#include <string>
#include <map>
#include <vector>

namespace perfsight {
namespace exporters {

/**
 * @brief Aggregated metrics from all plugins with metadata
 */
struct AggregatedMetrics {
    std::string deviceId;
    std::string location;
    std::string timestamp;
    std::map<std::string, plugins::MetricData> metricsByPlugin;
};

/**
 * @brief Interface for all exporters
 * 
 * Exporters are responsible for formatting and outputting collected metrics
 * in various formats (HTML, JSON, CSV, etc.)
 */
class IExporter {
public:
    virtual ~IExporter() = default;

    /**
     * @brief Initialize exporter with output directory
     * @param outputDir Directory where output files will be written
     * @return true if initialization successful
     */
    virtual bool initialize(const std::string& outputDir) = 0;

    /**
     * @brief Export aggregated metrics
     * @param metrics All collected metrics with metadata
     * @return true if export successful
     */
    virtual bool exportMetrics(const AggregatedMetrics& metrics) = 0;

    /**
     * @brief Get exporter type name
     */
    virtual std::string getType() const = 0;

    /**
     * @brief Cleanup resources
     */
    virtual void shutdown() = 0;
};

using ExporterPtr = std::shared_ptr<IExporter>;

} // namespace exporters
} // namespace perfsight
