#include "exporters/JSONExporter.hpp"
#include "core/Logger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>

namespace perfsight {
namespace exporters {

bool JSONExporter::initialize(const std::string& outputDir) {
    outputDir_ = outputDir;
    
    // Create output directory if it doesn't exist
    try {
        std::filesystem::create_directories(outputDir_);
        core::Logger::getInstance().info("JSON exporter initialized: ", outputDir_);
        return true;
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Failed to create output directory: ", e.what());
        return false;
    }
}

bool JSONExporter::exportMetrics(const AggregatedMetrics& metrics) {
    try {
        std::string json = generateJSON(metrics);
        std::string filename = outputDir_ + "/perfsight_metrics.json";
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            core::Logger::getInstance().error("Failed to open JSON file: ", filename);
            return false;
        }
        
        file << json;
        file.close();
        
        core::Logger::getInstance().debug("JSON metrics exported to: ", filename);
        return true;
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Failed to export JSON: ", e.what());
        return false;
    }
}

std::string JSONExporter::generateJSON(const AggregatedMetrics& metrics) {
    std::ostringstream json;
    
    json << "{\n";
    json << "  \"device_id\": \"" << escapeJSON(metrics.deviceId) << "\",\n";
    json << "  \"location\": \"" << escapeJSON(metrics.location) << "\",\n";
    json << "  \"timestamp\": \"" << escapeJSON(metrics.timestamp) << "\",\n";
    json << "  \"metrics\": {\n";
    
    size_t pluginCount = 0;
    for (const auto& [pluginName, metricData] : metrics.metricsByPlugin) {
        if (pluginCount > 0) json << ",\n";
        
        json << "    \"" << escapeJSON(pluginName) << "\": [\n";
        
        for (size_t i = 0; i < metricData.size(); ++i) {
            const auto& metric = metricData[i];
            json << "      {\n";
            json << "        \"name\": \"" << escapeJSON(metric.name) << "\",\n";
            json << "        \"value\": \"" << escapeJSON(metric.value) << "\",\n";
            json << "        \"unit\": \"" << escapeJSON(metric.unit) << "\",\n";
            json << "        \"description\": \"" << escapeJSON(metric.description) << "\"\n";
            json << "      }";
            
            if (i < metricData.size() - 1) {
                json << ",";
            }
            json << "\n";
        }
        
        json << "    ]";
        pluginCount++;
    }
    
    json << "\n  }\n";
    json << "}\n";
    
    return json.str();
}

std::string JSONExporter::escapeJSON(const std::string& data) {
    std::string escaped;
    escaped.reserve(data.size());
    
    for (char c : data) {
        switch (c) {
            case '\"': escaped.append("\\\""); break;
            case '\\': escaped.append("\\\\"); break;
            case '\b': escaped.append("\\b");  break;
            case '\f': escaped.append("\\f");  break;
            case '\n': escaped.append("\\n");  break;
            case '\r': escaped.append("\\r");  break;
            case '\t': escaped.append("\\t");  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    escaped.append(buf);
                } else {
                    escaped.push_back(c);
                }
                break;
        }
    }
    
    return escaped;
}

void JSONExporter::shutdown() {
    // Nothing to cleanup
}

} // namespace exporters
} // namespace perfsight
