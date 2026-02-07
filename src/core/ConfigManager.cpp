#include "core/ConfigManager.hpp"
#include "core/Logger.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace perfsight {
namespace core {

bool ConfigManager::loadConfig(const std::string& configPath) {
    auto& logger = Logger::getInstance();
    logger.info("Loading configuration from: ", configPath);

    std::ifstream file(configPath);
    if (!file.is_open()) {
        logger.error("Failed to open config file: ", configPath);
        return false;
    }

    std::string line;
    std::string currentSection;
    MetricConfig currentMetric;
    ExporterConfig currentExporter;
    bool inMetric = false;
    bool inExporter = false;
    int indent = 0;

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        std::string trimmedLine = trim(line);
        if (trimmedLine.empty() || trimmedLine[0] == '#') {
            continue;
        }

        // Count leading spaces for indentation
        int currentIndent = 0;
        for (char c : line) {
            if (c == ' ') currentIndent++;
            else break;
        }

        // Parse agent section
        if (trimmedLine.find("agent:") == 0) {
            currentSection = "agent";
            continue;
        }
        
        if (currentSection == "agent" && currentIndent == 2) {
            if (trimmedLine.find("device_id:") == 0) {
                agentConfig_.deviceId = extractValue(trimmedLine);
            } else if (trimmedLine.find("location:") == 0) {
                agentConfig_.location = extractValue(trimmedLine);
            } else if (trimmedLine.find("log_level:") == 0) {
                agentConfig_.logLevel = extractValue(trimmedLine);
            } else if (trimmedLine.find("collection_interval_seconds:") == 0) {
                agentConfig_.collectionIntervalSeconds = extractInt(trimmedLine);
            } else if (trimmedLine.find("collection_iteration:") == 0) {
                agentConfig_.collectionIteration = extractInt(trimmedLine);
            } else if (trimmedLine.find("max_parallel_plugins:") == 0) {
                agentConfig_.maxParallelPlugins = extractInt(trimmedLine);
            } else if (trimmedLine.find("restart_plugins_on_failure:") == 0) {
                agentConfig_.restartPluginsOnFailure = extractBool(trimmedLine);
            } else if (trimmedLine.find("plugin_isolation:") == 0) {
                agentConfig_.pluginIsolation = extractBool(trimmedLine);
            }
        }

        // Parse metrics section
        if (trimmedLine.find("metrics:") == 0) {
            currentSection = "metrics";
            continue;
        }
        
        if (currentSection == "metrics") {
            if (currentIndent == 2 && trimmedLine.find("- name:") == 0) {
                // Save previous metric if any
                if (inMetric) {
                    metrics_.push_back(currentMetric);
                }
                // Start new metric
                currentMetric = MetricConfig();
                currentMetric.name = extractValue(trimmedLine);
                inMetric = true;
            } else if (inMetric && currentIndent == 4) {
                if (trimmedLine.find("enabled:") == 0) {
                    currentMetric.enabled = extractBool(trimmedLine);
                } else if (trimmedLine.find("plugin:") == 0) {
                    currentMetric.plugin = extractValue(trimmedLine);
                }
            } else if (inMetric && currentIndent == 6) {
                // Parse params
                size_t colonPos = trimmedLine.find(':');
                if (colonPos != std::string::npos) {
                    std::string key = trim(trimmedLine.substr(0, colonPos));
                    std::string value = trim(trimmedLine.substr(colonPos + 1));
                    currentMetric.params[key] = value;
                }
            }
        }

        // Parse exporters section
        if (trimmedLine.find("exporters:") == 0) {
            // Save last metric if any
            if (inMetric) {
                metrics_.push_back(currentMetric);
                inMetric = false;
            }
            currentSection = "exporters";
            continue;
        }
        
        if (currentSection == "exporters") {
            if (currentIndent == 2 && trimmedLine.find("- type:") == 0) {
                // Save previous exporter if any
                if (inExporter) {
                    exporters_.push_back(currentExporter);
                }
                // Start new exporter
                currentExporter = ExporterConfig();
                currentExporter.type = extractValue(trimmedLine);
                inExporter = true;
            } else if (inExporter && currentIndent == 4) {
                if (trimmedLine.find("enabled:") == 0) {
                    currentExporter.enabled = extractBool(trimmedLine);
                } else if (trimmedLine.find("output_dir:") == 0) {
                    currentExporter.outputDir = extractValue(trimmedLine);
                }
            }
        }
    }

    // Save last metric/exporter if any
    if (inMetric) {
        metrics_.push_back(currentMetric);
    }
    if (inExporter) {
        exporters_.push_back(currentExporter);
    }

    logger.info("Configuration loaded successfully");
    logger.info("  Device ID: ", agentConfig_.deviceId);
    logger.info("  Metrics configured: ", metrics_.size());
    logger.info("  Exporters configured: ", exporters_.size());
    logger.info("  Collection iterations: ", 
                agentConfig_.collectionIteration == 0 ? "infinite" : std::to_string(agentConfig_.collectionIteration));

    return true;
}

std::string ConfigManager::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::string ConfigManager::extractValue(const std::string& line) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) return "";
    
    std::string value = trim(line.substr(colonPos + 1));
    
    // Remove quotes if present
    if (!value.empty() && (value.front() == '"' || value.front() == '\'')) {
        value = value.substr(1, value.length() - 2);
    }
    
    return value;
}

int ConfigManager::extractInt(const std::string& line) {
    std::string value = extractValue(line);
    try {
        return std::stoi(value);
    } catch (...) {
        return 0;
    }
}

bool ConfigManager::extractBool(const std::string& line) {
    std::string value = extractValue(line);
    return (value == "true" || value == "True" || value == "TRUE");
}

} // namespace core
} // namespace perfsight
