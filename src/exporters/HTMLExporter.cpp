#include "exporters/HTMLExporter.hpp"
#include "core/Logger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>

namespace perfsight {
namespace exporters {

bool HTMLExporter::initialize(const std::string& outputDir) {
    outputDir_ = outputDir;
    
    // Create output directory if it doesn't exist
    try {
        std::filesystem::create_directories(outputDir_);
        core::Logger::getInstance().info("HTML exporter initialized: ", outputDir_);
        return true;
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Failed to create output directory: ", e.what());
        return false;
    }
}

bool HTMLExporter::exportMetrics(const AggregatedMetrics& metrics) {
    try {
        std::string html = generateHTML(metrics);
        std::string filename = outputDir_ + "/perfsight_report.html";
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            core::Logger::getInstance().error("Failed to open HTML file: ", filename);
            return false;
        }
        
        file << html;
        file.close();
        
        core::Logger::getInstance().debug("HTML report exported to: ", filename);
        return true;
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Failed to export HTML: ", e.what());
        return false;
    }
}

std::string HTMLExporter::generateHTML(const AggregatedMetrics& metrics) {
    std::ostringstream html;
    
    html << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>perfSight Performance Report</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background-color: white;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            border-bottom: 3px solid #4CAF50;
            padding-bottom: 10px;
        }
        h2 {
            color: #555;
            margin-top: 30px;
            border-bottom: 2px solid #ddd;
            padding-bottom: 8px;
        }
        .metadata {
            background-color: #f9f9f9;
            padding: 15px;
            border-radius: 5px;
            margin: 20px 0;
        }
        .metadata-item {
            display: inline-block;
            margin-right: 30px;
            margin-bottom: 10px;
        }
        .metadata-label {
            font-weight: bold;
            color: #666;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
        }
        th {
            background-color: #4CAF50;
            color: white;
            padding: 12px;
            text-align: left;
            font-weight: 600;
        }
        td {
            padding: 10px 12px;
            border-bottom: 1px solid #ddd;
        }
        tr:hover {
            background-color: #f5f5f5;
        }
        .no-data {
            color: #999;
            font-style: italic;
            padding: 20px;
            text-align: center;
        }
        .footer {
            margin-top: 40px;
            padding-top: 20px;
            border-top: 1px solid #ddd;
            color: #999;
            font-size: 0.9em;
            text-align: center;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔍 perfSight Performance Report</h1>
        
        <div class="metadata">
            <div class="metadata-item">
                <span class="metadata-label">Device ID:</span> )";
    
    html << escapeHTML(metrics.deviceId) << R"(
            </div>
            <div class="metadata-item">
                <span class="metadata-label">Location:</span> )";
    
    html << escapeHTML(metrics.location) << R"(
            </div>
            <div class="metadata-item">
                <span class="metadata-label">Timestamp:</span> )";
    
    html << escapeHTML(metrics.timestamp) << R"(
            </div>
        </div>
)";

    // Generate tables for each plugin
    for (const auto& [pluginName, metricData] : metrics.metricsByPlugin) {
        html << "        <h2>" << escapeHTML(pluginName) << "</h2>\n";
        
        if (metricData.empty()) {
            html << "        <div class=\"no-data\">No metrics collected</div>\n";
        } else {
            html << R"(        <table>
            <thead>
                <tr>
                    <th>Metric</th>
                    <th>Value</th>
                    <th>Unit</th>
                    <th>Description</th>
                </tr>
            </thead>
            <tbody>
)";
            
            for (const auto& metric : metricData) {
                html << "                <tr>\n";
                html << "                    <td>" << escapeHTML(metric.name) << "</td>\n";
                html << "                    <td>" << escapeHTML(metric.value) << "</td>\n";
                html << "                    <td>" << escapeHTML(metric.unit) << "</td>\n";
                html << "                    <td>" << escapeHTML(metric.description) << "</td>\n";
                html << "                </tr>\n";
            }
            
            html << R"(            </tbody>
        </table>
)";
        }
    }
    
    html << R"(        
        <div class="footer">
            Generated by perfSight - Lightweight Performance Monitoring Tool
        </div>
    </div>
</body>
</html>
)";
    
    return html.str();
}

std::string HTMLExporter::escapeHTML(const std::string& data) {
    std::string escaped;
    escaped.reserve(data.size());
    
    for (char c : data) {
        switch (c) {
            case '&':  escaped.append("&amp;");  break;
            case '\"': escaped.append("&quot;"); break;
            case '\'': escaped.append("&#39;");  break;
            case '<':  escaped.append("&lt;");   break;
            case '>':  escaped.append("&gt;");   break;
            default:   escaped.push_back(c);     break;
        }
    }
    
    return escaped;
}

void HTMLExporter::shutdown() {
    // Nothing to cleanup
}

} // namespace exporters
} // namespace perfsight
