#pragma once

#include <string>
#include <vector>
#include <map>

namespace perfsight {
namespace core {

struct MetricConfig {
    std::string name;
    bool enabled;
    std::string plugin;
    std::map<std::string, std::string> params;
};

struct ExporterConfig {
    std::string type;
    bool enabled;
    std::string outputDir;
};

struct AgentConfig {
    std::string deviceId;
    std::string location;
    std::string logLevel;
    int collectionIntervalSeconds;
    int collectionIteration;  // Number of iterations (0 = infinite)
    int maxParallelPlugins;
    bool restartPluginsOnFailure;
    bool pluginIsolation;
};

/**
 * @brief Lightweight YAML configuration parser
 */
class ConfigManager {
public:
    ConfigManager() = default;
    
    bool loadConfig(const std::string& configPath);
    
    const AgentConfig& getAgentConfig() const { return agentConfig_; }
    const std::vector<MetricConfig>& getMetrics() const { return metrics_; }
    const std::vector<ExporterConfig>& getExporters() const { return exporters_; }

private:
    AgentConfig agentConfig_;
    std::vector<MetricConfig> metrics_;
    std::vector<ExporterConfig> exporters_;
    
    // Simple YAML parsing helpers
    std::string trim(const std::string& str);
    std::string extractValue(const std::string& line);
    int extractInt(const std::string& line);
    bool extractBool(const std::string& line);
};

} // namespace core
} // namespace perfsight
