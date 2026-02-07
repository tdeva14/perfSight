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
        metrics = collectProcessMetrics();
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Exception in ProcMetricsPlugin: ", e.what());
        healthy_ = false;
    }

    return metrics;
}

MetricData ProcMetricsPlugin::collectProcessMetrics() {
    MetricData metrics;
    auto processList = getProcessList();

    for (const auto& pid : processList) {
        try {
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

            // Collect metrics from different sources
            parseProcessStat(pid, processName, metrics);
            parseProcessStatus(pid, processName, metrics);
            parseProcessSmaps(pid, processName, metrics);
            
        } catch (...) {
            // Process may have exited, continue
        }
    }

    return metrics;
}

void ProcMetricsPlugin::parseProcessStat(const std::string& pid, const std::string& processName, MetricData& metrics) {
    std::string statPath = "/proc/" + pid + "/stat";
    std::string stat = readFile(statPath);
    
    if (stat.empty()) return;
    
    std::istringstream iss(stat);
    std::string pidStr, comm, state;
    long ppid = 0, pgrp = 0, session = 0, tty_nr = 0, tpgid = 0, flags = 0;
    unsigned long minflt = 0, cminflt = 0, majflt = 0, cmajflt = 0, utime = 0, stime = 0;
    long cutime = 0, cstime = 0, priority = 0, nice = 0, num_threads = 0, itrealvalue = 0;
    unsigned long long starttime = 0, vsize = 0;
    long rss = 0;
    
    // Parse stat file (man proc)
    iss >> pidStr >> comm >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid 
        >> flags >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime
        >> cutime >> cstime >> priority >> nice >> num_threads >> itrealvalue
        >> starttime >> vsize >> rss;
    
    // Only create metrics if parsing was successful
    if (!iss.fail() || iss.eof()) {
        std::string prefix = processName + "_" + pid;
        
        metrics.push_back(MetricValue(prefix + "_state", state, "", "Process state"));
        metrics.push_back(MetricValue(prefix + "_utime", std::to_string(utime), "jiffies", "User CPU time"));
        metrics.push_back(MetricValue(prefix + "_stime", std::to_string(stime), "jiffies", "System CPU time"));
        metrics.push_back(MetricValue(prefix + "_num_threads", std::to_string(num_threads), "", "Number of threads"));
        metrics.push_back(MetricValue(prefix + "_vsize", std::to_string(vsize), "bytes", "Virtual memory size"));
        metrics.push_back(MetricValue(prefix + "_rss", std::to_string(rss), "pages", "Resident set size"));
    }
}

void ProcMetricsPlugin::parseProcessStatus(const std::string& pid, const std::string& processName, MetricData& metrics) {
    std::string statusPath = "/proc/" + pid + "/status";
    std::string status = readFile(statusPath);
    
    if (status.empty()) return;
    
    std::istringstream iss(status);
    std::string line;
    std::string prefix = processName + "_" + pid;
    
    while (std::getline(iss, line)) {
        if (line.find("VmRSS:") == 0) {
            std::istringstream lineStream(line);
            std::string key, value, unit;
            lineStream >> key >> value >> unit;
            metrics.push_back(MetricValue(prefix + "_VmRSS", value, unit, "Resident memory"));
        } else if (line.find("VmSize:") == 0) {
            std::istringstream lineStream(line);
            std::string key, value, unit;
            lineStream >> key >> value >> unit;
            metrics.push_back(MetricValue(prefix + "_VmSize", value, unit, "Virtual memory size"));
        } else if (line.find("VmPeak:") == 0) {
            std::istringstream lineStream(line);
            std::string key, value, unit;
            lineStream >> key >> value >> unit;
            metrics.push_back(MetricValue(prefix + "_VmPeak", value, unit, "Peak virtual memory"));
        } else if (line.find("VmHWM:") == 0) {
            std::istringstream lineStream(line);
            std::string key, value, unit;
            lineStream >> key >> value >> unit;
            metrics.push_back(MetricValue(prefix + "_VmHWM", value, unit, "Peak resident memory"));
        }
    }
}

void ProcMetricsPlugin::parseProcessSmaps(const std::string& pid, const std::string& processName, MetricData& metrics) {
    std::string smapsPath = "/proc/" + pid + "/smaps";
    std::string smaps = readFile(smapsPath);
    
    if (smaps.empty()) return;
    
    std::istringstream iss(smaps);
    std::string line;
    std::string prefix = processName + "_" + pid;
    
    long totalPss = 0;
    long totalPrivateClean = 0;
    long totalPrivateDirty = 0;
    long totalSharedClean = 0;
    long totalSharedDirty = 0;
    
    while (std::getline(iss, line)) {
        if (line.find("Pss:") == 0) {
            std::istringstream lineStream(line);
            std::string key, value;
            lineStream >> key >> value;
            totalPss += std::stol(value);
        } else if (line.find("Private_Clean:") == 0) {
            std::istringstream lineStream(line);
            std::string key, value;
            lineStream >> key >> value;
            totalPrivateClean += std::stol(value);
        } else if (line.find("Private_Dirty:") == 0) {
            std::istringstream lineStream(line);
            std::string key, value;
            lineStream >> key >> value;
            totalPrivateDirty += std::stol(value);
        } else if (line.find("Shared_Clean:") == 0) {
            std::istringstream lineStream(line);
            std::string key, value;
            lineStream >> key >> value;
            totalSharedClean += std::stol(value);
        } else if (line.find("Shared_Dirty:") == 0) {
            std::istringstream lineStream(line);
            std::string key, value;
            lineStream >> key >> value;
            totalSharedDirty += std::stol(value);
        }
    }
    
    metrics.push_back(MetricValue(prefix + "_Pss", std::to_string(totalPss), "kB", "Proportional set size"));
    metrics.push_back(MetricValue(prefix + "_Private_Clean", std::to_string(totalPrivateClean), "kB", "Private clean memory"));
    metrics.push_back(MetricValue(prefix + "_Private_Dirty", std::to_string(totalPrivateDirty), "kB", "Private dirty memory"));
    metrics.push_back(MetricValue(prefix + "_Shared_Clean", std::to_string(totalSharedClean), "kB", "Shared clean memory"));
    metrics.push_back(MetricValue(prefix + "_Shared_Dirty", std::to_string(totalSharedDirty), "kB", "Shared dirty memory"));
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
