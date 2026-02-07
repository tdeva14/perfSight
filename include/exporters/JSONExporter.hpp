#pragma once

#include "IExporter.hpp"
#include <string>

namespace perfsight {
namespace exporters {

/**
 * @brief JSON exporter
 * 
 * Exports metrics in JSON format for easy parsing and integration
 */
class JSONExporter : public IExporter {
public:
    JSONExporter() = default;
    ~JSONExporter() override = default;

    bool initialize(const std::string& outputDir) override;
    bool exportMetrics(const AggregatedMetrics& metrics) override;
    std::string getType() const override { return "json"; }
    void shutdown() override;

private:
    std::string outputDir_;

    std::string generateJSON(const AggregatedMetrics& metrics);
    std::string escapeJSON(const std::string& data);
};

} // namespace exporters
} // namespace perfsight
