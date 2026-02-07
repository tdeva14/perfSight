#include "plugins/SystemMetricsPlugin.hpp"
#include "core/Logger.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace perfsight {
namespace plugins {

SystemMetricsPlugin::SystemMetricsPlugin(const std::string& metricName)
    : metricName_(metricName), healthy_(true) {
}

bool SystemMetricsPlugin::initialize(const PluginParams& params) {
    auto& logger = core::Logger::getInstance();
    logger.debug("Initializing SystemMetricsPlugin: ", metricName_);
    healthy_ = true;
    return true;
}

MetricData SystemMetricsPlugin::collectMetrics() {
    MetricData metrics;

    try {
        // Collect all system metrics
        auto memMetrics = collectSystemMemory();
        metrics.insert(metrics.end(), memMetrics.begin(), memMetrics.end());
        
        auto cpuMetrics = collectCPUUsage();
        metrics.insert(metrics.end(), cpuMetrics.begin(), cpuMetrics.end());
        
        auto buddyMetrics = collectBuddyInfo();
        metrics.insert(metrics.end(), buddyMetrics.begin(), buddyMetrics.end());
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Exception in SystemMetricsPlugin: ", e.what());
        healthy_ = false;
    }

    return metrics;
}

MetricData SystemMetricsPlugin::collectSystemMemory() {
    MetricData metrics;
    std::string meminfo = readFile("/proc/meminfo");
    std::istringstream iss(meminfo);
    std::string line;

    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        std::string key, value, unit;
        lineStream >> key >> value >> unit;

        if (key == "MemTotal:") {
            metrics.push_back(MetricValue("MemTotal", value, "kB", "Total system memory"));
        } else if (key == "MemFree:") {
            metrics.push_back(MetricValue("MemFree", value, "kB", "Free system memory"));
        } else if (key == "MemAvailable:") {
            metrics.push_back(MetricValue("MemAvailable", value, "kB", "Available system memory"));
        } else if (key == "Buffers:") {
            metrics.push_back(MetricValue("Buffers", value, "kB", "Buffer cache"));
        } else if (key == "Cached:") {
            metrics.push_back(MetricValue("Cached", value, "kB", "Page cache"));
        } else if (key == "SwapTotal:") {
            metrics.push_back(MetricValue("SwapTotal", value, "kB", "Total swap space"));
        } else if (key == "SwapFree:") {
            metrics.push_back(MetricValue("SwapFree", value, "kB", "Free swap space"));
        } else if (key == "SwapCached:") {
            metrics.push_back(MetricValue("SwapCached", value, "kB", "Swap cached"));
        } else if (key == "Active:") {
            metrics.push_back(MetricValue("Active", value, "kB", "Active memory"));
        } else if (key == "Inactive:") {
            metrics.push_back(MetricValue("Inactive", value, "kB", "Inactive memory"));
        }
    }

    return metrics;
}

MetricData SystemMetricsPlugin::collectCPUUsage() {
    MetricData metrics;
    std::string stat = readFile("/proc/stat");
    std::istringstream iss(stat);
    std::string line;

    std::getline(iss, line); // First line is overall CPU
    std::istringstream lineStream(line);
    std::string cpu;
    long user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0;
    long steal = 0, guest = 0, guest_nice = 0;
    
    lineStream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
    
    // Try to read steal, guest, guest_nice (may not exist on all systems)
    lineStream >> steal >> guest >> guest_nice;

    metrics.push_back(MetricValue("cpu_user", std::to_string(user), "jiffies", "User mode CPU time"));
    metrics.push_back(MetricValue("cpu_nice", std::to_string(nice), "jiffies", "Nice user mode CPU time"));
    metrics.push_back(MetricValue("cpu_system", std::to_string(system), "jiffies", "System mode CPU time"));
    metrics.push_back(MetricValue("cpu_idle", std::to_string(idle), "jiffies", "Idle CPU time"));
    metrics.push_back(MetricValue("cpu_iowait", std::to_string(iowait), "jiffies", "I/O wait time"));
    metrics.push_back(MetricValue("cpu_irq", std::to_string(irq), "jiffies", "IRQ time"));
    metrics.push_back(MetricValue("cpu_softirq", std::to_string(softirq), "jiffies", "Soft IRQ time"));

    return metrics;
}

MetricData SystemMetricsPlugin::collectBuddyInfo() {
    MetricData metrics;
    std::string buddyinfo = readFile("/proc/buddyinfo");
    
    if (buddyinfo.empty()) {
        return metrics; // buddyinfo may not be available on all systems
    }
    
    std::istringstream iss(buddyinfo);
    std::string line;
    int zoneCount = 0;

    while (std::getline(iss, line)) {
        // Format: Node 0, zone DMA 1 1 1 0 2 1 1 0 1 1 3
        std::istringstream lineStream(line);
        std::string node, nodeNum, comma, zone, zoneName;
        
        lineStream >> node >> nodeNum >> comma >> zone >> zoneName;
        
        // Read order counts
        std::vector<std::string> orderCounts;
        std::string count;
        while (lineStream >> count) {
            orderCounts.push_back(count);
        }
        
        // Create metric for this zone
        std::string metricName = "buddyinfo_" + zoneName + "_orders";
        std::string value;
        for (size_t i = 0; i < orderCounts.size(); ++i) {
            if (i > 0) value += ",";
            value += orderCounts[i];
        }
        
        metrics.push_back(MetricValue(metricName, value, "", 
                                     "Free pages per order for " + zoneName + " zone"));
        zoneCount++;
    }

    return metrics;
}

std::string SystemMetricsPlugin::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void SystemMetricsPlugin::shutdown() {
    healthy_ = false;
}

} // namespace plugins
} // namespace perfsight
