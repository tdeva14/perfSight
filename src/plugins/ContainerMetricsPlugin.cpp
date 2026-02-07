#include "plugins/ContainerMetricsPlugin.hpp"
#include "core/Logger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace perfsight {
namespace plugins {

ContainerMetricsPlugin::ContainerMetricsPlugin(const std::string& metricName)
    : metricName_(metricName), healthy_(true) {
}

bool ContainerMetricsPlugin::initialize(const PluginParams& params) {
    auto& logger = core::Logger::getInstance();
    logger.debug("Initializing ContainerMetricsPlugin: ", metricName_);
    
    healthy_ = true;
    return true;
}

MetricData ContainerMetricsPlugin::collectMetrics() {
    MetricData metrics;

    try {
        if (metricName_ == "container_memory") {
            metrics = collectContainerMemory();
        }
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Exception in ContainerMetricsPlugin: ", e.what());
        healthy_ = false;
    }

    return metrics;
}

MetricData ContainerMetricsPlugin::collectContainerMemory() {
    MetricData metrics;
    auto containers = getContainerList();

    for (const auto& containerId : containers) {
        try {
            // Read cgroup memory stats
            // Try cgroup v2 first, then fall back to v1
            std::string cgroupV2Path = "/sys/fs/cgroup/" + containerId + "/memory.current";
            std::string cgroupV1Path = "/sys/fs/cgroup/memory/docker/" + containerId + "/memory.usage_in_bytes";
            
            std::string memUsage = readFile(cgroupV2Path);
            std::string cgroupVersion = "v2";
            
            if (memUsage.empty()) {
                memUsage = readFile(cgroupV1Path);
                cgroupVersion = "v1";
            }
            
            if (!memUsage.empty()) {
                // Remove newline
                memUsage.erase(std::remove(memUsage.begin(), memUsage.end(), '\n'), memUsage.end());
                
                std::string shortId = containerId.substr(0, 12);
                metrics.push_back(MetricValue("container_" + shortId + "_memory", 
                                             memUsage, "bytes", 
                                             "Container memory usage (cgroup " + cgroupVersion + ")"));
            }
            
            // Also try to get memory limit
            std::string limitPath = cgroupVersion == "v2" ? 
                "/sys/fs/cgroup/" + containerId + "/memory.max" :
                "/sys/fs/cgroup/memory/docker/" + containerId + "/memory.limit_in_bytes";
            
            std::string memLimit = readFile(limitPath);
            if (!memLimit.empty()) {
                memLimit.erase(std::remove(memLimit.begin(), memLimit.end(), '\n'), memLimit.end());
                std::string shortId = containerId.substr(0, 12);
                metrics.push_back(MetricValue("container_" + shortId + "_memory_limit", 
                                             memLimit, "bytes", "Container memory limit"));
            }
            
        } catch (...) {
            // Container may have stopped, continue
        }
    }

    if (metrics.empty()) {
        metrics.push_back(MetricValue("container_info", "no_containers_found", "", 
                                     "No running containers detected"));
    }

    return metrics;
}

std::string ContainerMetricsPlugin::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> ContainerMetricsPlugin::getContainerList() {
    std::vector<std::string> containers;
    
    // Try to find containers in cgroup v2
    std::string cgroupV2Base = "/sys/fs/cgroup";
    try {
        if (std::filesystem::exists(cgroupV2Base)) {
            for (const auto& entry : std::filesystem::directory_iterator(cgroupV2Base)) {
                if (entry.is_directory()) {
                    std::string name = entry.path().filename().string();
                    // Container IDs are typically long hex strings
                    if (name.length() == 64 && 
                        std::all_of(name.begin(), name.end(), 
                                  [](char c) { return std::isxdigit(c); })) {
                        containers.push_back(name);
                    }
                }
            }
        }
    } catch (...) {
        // Ignore errors
    }
    
    // Try cgroup v1 if v2 didn't find anything
    if (containers.empty()) {
        std::string cgroupV1Base = "/sys/fs/cgroup/memory/docker";
        try {
            if (std::filesystem::exists(cgroupV1Base)) {
                for (const auto& entry : std::filesystem::directory_iterator(cgroupV1Base)) {
                    if (entry.is_directory()) {
                        std::string name = entry.path().filename().string();
                        if (name.length() == 64) {
                            containers.push_back(name);
                        }
                    }
                }
            }
        } catch (...) {
            // Ignore errors
        }
    }
    
    return containers;
}

void ContainerMetricsPlugin::shutdown() {
    healthy_ = false;
}

} // namespace plugins
} // namespace perfsight
