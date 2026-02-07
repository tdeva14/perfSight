#pragma once

#include "IExporter.hpp"
#include <string>

namespace perfsight {
namespace exporters {

/**
 * @brief CSV exporter
 * 
 * Exports metrics in CSV format for spreadsheet analysis
 */
class CSVExporter : public IExporter {
public:
    CSVExporter() = default;
    ~CSVExporter() override = default;

    bool initialize(const std::string& outputDir) override;
    bool exportMetrics(const AggregatedMetrics& metrics) override;
    std::string getType() const override { return "csv"; }
    void shutdown() override;

private:
    std::string outputDir_;

    std::string generateCSV(const AggregatedMetrics& metrics);
    std::string escapeCSV(const std::string& data);
};

} // namespace exporters
} // namespace perfsight
