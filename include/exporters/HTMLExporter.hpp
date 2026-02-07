#pragma once

#include "IExporter.hpp"
#include <string>

namespace perfsight {
namespace exporters {

/**
 * @brief HTML report exporter
 * 
 * Generates a formatted HTML report with metrics displayed in tables
 */
class HTMLExporter : public IExporter {
public:
    HTMLExporter() = default;
    ~HTMLExporter() override = default;

    bool initialize(const std::string& outputDir) override;
    bool exportMetrics(const AggregatedMetrics& metrics) override;
    std::string getType() const override { return "html_report"; }
    void shutdown() override;

private:
    std::string outputDir_;

    std::string generateHTML(const AggregatedMetrics& metrics);
    std::string escapeHTML(const std::string& data);
};

} // namespace exporters
} // namespace perfsight
