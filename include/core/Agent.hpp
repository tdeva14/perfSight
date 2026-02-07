#pragma once

#include "core/ConfigManager.hpp"
#include "core/MetricsAggregator.hpp"
#include "exporters/IExporter.hpp"
#include <memory>
#include <vector>
#include <atomic>

namespace perfsight {
namespace core {

/**
 * @brief Main agent orchestrator
 */
class Agent {
public:
    Agent();
    ~Agent();
    
    bool initialize(const std::string& configPath);
    void run();
    void shutdown();

private:
    ConfigManager configManager_;
    std::unique_ptr<MetricsAggregator> aggregator_;
    std::vector<std::shared_ptr<exporters::IExporter>> exporters_;
    
    std::atomic<bool> running_;
    int iterationCount_;
    int maxIterations_;
    
    void signalHandler(int signal);
    void setupSignalHandlers();
    bool createExporters();
};

} // namespace core
} // namespace perfsight
