#include "plugins/ProcMetricsPlugin.hpp"
#include "core/Logger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace perfsight {
namespace plugins {

ProcMetricsPlugin::ProcMetricsPlugin(const std::string& metricName)
    : metricName_(metricName), healthy_(true), includeKernelThreads_(false) {
}

bool ProcMetricsPlugin::initialize(const PluginParams& params) {
    auto& logger = core::Logger::getInstance();
    logger.debug("Initializing ProcMetricsPlugin: ", metricName_);

    // Parse process whitelist
    auto it = params.find("process_whitelist");
    if (it != params.end()) {
        std::istringstream iss(it->second);
        std::string process;
        while (std::getline(iss, process, ',')) {
            // Remove brackets and quotes
            process.erase(std::remove(process.begin(), process.end(), '['), process.end());
            process.erase(std::remove(process.begin(), process.end(), ']'), process.end());
            process.erase(std::remove(process.begin(), process.end(), '"'), process.end());
            process.erase(std::remove(process.begin(), process.end(), ' '), process.end());
            if (!process.empty()) {
                processWhitelist_.insert(process);
            }
        }
    }

    // Parse include_kernel_threads
    it = params.find("include_kernel_threads");
    if (it != params.end()) {
        includeKernelThreads_ = (it->second == "true");
    }

    healthy_ = true;
    return true;
}

MetricData ProcMetricsPlugin::collectMetrics() {
    MetricData metrics;

    try {
        if (metricName_ == "system_memory") {
            metrics = collectSystemMemory();
        } else if (metricName_ == "process_memory") {
            metrics = collectProcessMemory();
        } else if (metricName_ == "cpu_usage") {
            metrics = collectCPUUsage();
        }
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Exception in ProcMetricsPlugin: ", e.what());
        healthy_ = false;
    }

    return metrics;
}

MetricData ProcMetricsPlugin::collectSystemMemory() {
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
        }
    }

    return metrics;
}

MetricData ProcMetricsPlugin::collectProcessMemory() {
    MetricData metrics;
    auto processList = getProcessList();

    for (const auto& pid : processList) {
        try {
            std::string statusPath = "/proc/" + pid + "/status";
            std::string cmdlinePath = "/proc/" + pid + "/cmdline";
            
            // Read process name
            std::string cmdline = readFile(cmdlinePath);
            if (cmdline.empty()) continue;
            
            // Extract process name
            size_t nullPos = cmdline.find('\0');
            std::string processName = cmdline.substr(0, nullPos);
            size_t lastSlash = processName.find_last_of('/');
            if (lastSlash != std::string::npos) {
                processName = processName.substr(lastSlash + 1);
            }

            // Check whitelist
            if (!isProcessInWhitelist(processName)) continue;

            // Read memory stats
            std::string status = readFile(statusPath);
            std::istringstream iss(status);
            std::string line;
            
            while (std::getline(iss, line)) {
                if (line.find("VmRSS:") == 0) {
                    std::istringstream lineStream(line);
                    std::string key, value, unit;
                    lineStream >> key >> value >> unit;
                    
                    std::string metricName = processName + "_VmRSS_" + pid;
                    metrics.push_back(MetricValue(metricName, value, unit, 
                                                 "Resident memory for " + processName));
                } else if (line.find("VmSize:") == 0) {
                    std::istringstream lineStream(line);
                    std::string key, value, unit;
                    lineStream >> key >> value >> unit;
                    
                    std::string metricName = processName + "_VmSize_" + pid;
                    metrics.push_back(MetricValue(metricName, value, unit, 
                                                 "Virtual memory for " + processName));
                }
            }
        } catch (...) {
            // Process may have exited, continue
        }
    }

    return metrics;
}

MetricData ProcMetricsPlugin::collectCPUUsage() {
    MetricData metrics;
    std::string stat = readFile("/proc/stat");
    std::istringstream iss(stat);
    std::string line;

    std::getline(iss, line); // First line is overall CPU
    std::istringstream lineStream(line);
    std::string cpu;
    long user, nice, system, idle, iowait, irq, softirq;
    
    lineStream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;

    metrics.push_back(MetricValue("cpu_user", std::to_string(user), "jiffies", "User mode CPU time"));
    metrics.push_back(MetricValue("cpu_system", std::to_string(system), "jiffies", "System mode CPU time"));
    metrics.push_back(MetricValue("cpu_idle", std::to_string(idle), "jiffies", "Idle CPU time"));
    metrics.push_back(MetricValue("cpu_iowait", std::to_string(iowait), "jiffies", "I/O wait time"));

    return metrics;
}

std::string ProcMetricsPlugin::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> ProcMetricsPlugin::getProcessList() {
    std::vector<std::string> pids;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
            if (entry.is_directory()) {
                std::string filename = entry.path().filename().string();
                // Check if directory name is numeric (PID)
                if (!filename.empty() && std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                    pids.push_back(filename);
                }
            }
        }
    } catch (...) {
        // Ignore errors
    }
    
    return pids;
}

bool ProcMetricsPlugin::isProcessInWhitelist(const std::string& processName) {
    if (processWhitelist_.empty()) {
        return true; // No whitelist means all processes
    }
    return processWhitelist_.find(processName) != processWhitelist_.end();
}

void ProcMetricsPlugin::shutdown() {
    healthy_ = false;
}

} // namespace plugins
} // namespace perfsight
