#include "plugins/SoCMetricsPlugin.hpp"
#include "core/Logger.hpp"
#include <fstream>
#include <sstream>

namespace perfsight {
namespace plugins {

SoCMetricsPlugin::SoCMetricsPlugin(const std::string& metricName)
    : metricName_(metricName), socType_("generic"), healthy_(true) {
}

bool SoCMetricsPlugin::initialize(const PluginParams& params) {
    auto& logger = core::Logger::getInstance();
    logger.debug("Initializing SoCMetricsPlugin: ", metricName_);

    // Parse SoC type
    auto it = params.find("type");
    if (it != params.end()) {
        socType_ = it->second;
    }

    logger.info("SoCMetricsPlugin initialized for SoC type: ", socType_);
    healthy_ = true;
    return true;
}

MetricData SoCMetricsPlugin::collectMetrics() {
    MetricData metrics;

    try {
        if (metricName_ == "gpu_memory") {
            metrics = collectGPUMemory();
        } else if (metricName_ == "cma_regions") {
            metrics = collectCMARegions();
        } else if (metricName_ == "memory_bandwidth") {
            metrics = collectMemoryBandwidth();
        }
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Exception in SoCMetricsPlugin: ", e.what());
        healthy_ = false;
    }

    return metrics;
}

MetricData SoCMetricsPlugin::collectGPUMemory() {
    MetricData metrics;
    
    // Try different paths based on SoC type
    std::string gpuMemPath = getSoCSpecificPath("gpu_memory");
    std::string content = readFile(gpuMemPath);
    
    if (!content.empty()) {
        // Parse GPU memory info (format varies by SoC)
        std::istringstream iss(content);
        std::string line;
        
        while (std::getline(iss, line)) {
            // Generic parsing - in real implementation, this would be SoC-specific
            if (line.find("used") != std::string::npos || 
                line.find("allocated") != std::string::npos) {
                
                std::istringstream lineStream(line);
                std::string key, value;
                lineStream >> key >> value;
                
                metrics.push_back(MetricValue("gpu_memory_used", value, "bytes", 
                                             "GPU memory allocated"));
            }
        }
    } else {
        // Fallback - simulate GPU memory for demonstration
        metrics.push_back(MetricValue("gpu_memory_info", "unavailable", "", 
                                     "GPU memory information not available on this platform"));
    }
    
    return metrics;
}

MetricData SoCMetricsPlugin::collectCMARegions() {
    MetricData metrics;
    
    // CMA info is typically in /proc/cma or /sys/kernel/debug/cma
    std::string cmaPath = "/proc/cma";
    std::string content = readFile(cmaPath);
    
    if (content.empty()) {
        cmaPath = "/sys/kernel/debug/cma";
        content = readFile(cmaPath);
    }
    
    if (!content.empty()) {
        std::istringstream iss(content);
        std::string line;
        int regionCount = 0;
        
        while (std::getline(iss, line)) {
            if (line.find("region") != std::string::npos || 
                line.find("area") != std::string::npos) {
                regionCount++;
                metrics.push_back(MetricValue("cma_region_" + std::to_string(regionCount), 
                                             line, "", "CMA region information"));
            }
        }
    } else {
        metrics.push_back(MetricValue("cma_info", "unavailable", "", 
                                     "CMA information not available"));
    }
    
    return metrics;
}

MetricData SoCMetricsPlugin::collectMemoryBandwidth() {
    MetricData metrics;
    
    // Memory bandwidth monitoring is very SoC-specific
    std::string bwPath = getSoCSpecificPath("memory_bandwidth");
    std::string content = readFile(bwPath);
    
    if (!content.empty()) {
        // Parse bandwidth info
        metrics.push_back(MetricValue("memory_bandwidth", content, "MB/s", 
                                     "Memory bandwidth usage"));
    } else {
        // Fallback - try perf counters or other methods
        metrics.push_back(MetricValue("memory_bandwidth", "unavailable", "", 
                                     "Memory bandwidth monitoring not available on this platform"));
    }
    
    return metrics;
}

std::string SoCMetricsPlugin::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string SoCMetricsPlugin::getSoCSpecificPath(const std::string& metric) {
    // Map SoC types to their specific sysfs/debugfs paths
    if (socType_ == "amlogic") {
        if (metric == "gpu_memory") {
            return "/sys/class/misc/mali0/device/gpuinfo";
        } else if (metric == "memory_bandwidth") {
            return "/sys/class/aml_ddr/port_ctrl";
        }
    } else if (socType_ == "broadcom") {
        if (metric == "gpu_memory") {
            return "/sys/kernel/debug/vc4/gpu_mem";
        }
    } else if (socType_ == "realtek") {
        if (metric == "gpu_memory") {
            return "/sys/class/graphics/fb0/gpu_mem";
        }
    }
    
    // Generic fallback paths
    if (metric == "gpu_memory") {
        return "/sys/kernel/debug/dri/0/gem_objects";
    }
    
    return "";
}

void SoCMetricsPlugin::shutdown() {
    healthy_ = false;
}

} // namespace plugins
} // namespace perfsight
