#include "exporters/CSVExporter.hpp"
#include "core/Logger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace perfsight {
namespace exporters {

bool CSVExporter::initialize(const std::string& outputDir) {
    outputDir_ = outputDir;
    
    // Create output directory if it doesn't exist
    try {
        std::filesystem::create_directories(outputDir_);
        core::Logger::getInstance().info("CSV exporter initialized: ", outputDir_);
        return true;
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Failed to create output directory: ", e.what());
        return false;
    }
}

bool CSVExporter::exportMetrics(const AggregatedMetrics& metrics) {
    try {
        std::string csv = generateCSV(metrics);
        std::string filename = outputDir_ + "/perfsight_metrics.csv";
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            core::Logger::getInstance().error("Failed to open CSV file: ", filename);
            return false;
        }
        
        file << csv;
        file.close();
        
        core::Logger::getInstance().debug("CSV metrics exported to: ", filename);
        return true;
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Failed to export CSV: ", e.what());
        return false;
    }
}

std::string CSVExporter::generateCSV(const AggregatedMetrics& metrics) {
    std::ostringstream csv;
    
    // CSV Header
    csv << "Device ID,Location,Timestamp,Plugin,Metric Name,Value,Unit,Description\n";
    
    // Data rows
    for (const auto& [pluginName, metricData] : metrics.metricsByPlugin) {
        for (const auto& metric : metricData) {
            csv << escapeCSV(metrics.deviceId) << ",";
            csv << escapeCSV(metrics.location) << ",";
            csv << escapeCSV(metrics.timestamp) << ",";
            csv << escapeCSV(pluginName) << ",";
            csv << escapeCSV(metric.name) << ",";
            csv << escapeCSV(metric.value) << ",";
            csv << escapeCSV(metric.unit) << ",";
            csv << escapeCSV(metric.description) << "\n";
        }
    }
    
    return csv.str();
}

std::string CSVExporter::escapeCSV(const std::string& data) {
    // Check if escaping is needed
    bool needsQuotes = false;
    for (char c : data) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            needsQuotes = true;
            break;
        }
    }
    
    if (!needsQuotes) {
        return data;
    }
    
    // Escape quotes and wrap in quotes
    std::string escaped = "\"";
    for (char c : data) {
        if (c == '"') {
            escaped.append("\"\"");
        } else {
            escaped.push_back(c);
        }
    }
    escaped.push_back('"');
    
    return escaped;
}

void CSVExporter::shutdown() {
    // Nothing to cleanup
}

} // namespace exporters
} // namespace perfsight
