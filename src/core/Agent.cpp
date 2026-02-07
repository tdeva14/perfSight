#include "core/Agent.hpp"
#include "core/Logger.hpp"
#include "core/PluginLoader.hpp"
#include "exporters/HTMLExporter.hpp"
#include "exporters/JSONExporter.hpp"
#include "exporters/CSVExporter.hpp"
#include <csignal>
#include <thread>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace perfsight {
namespace core {

// Global pointer for signal handler
static Agent* g_agent = nullptr;

void signalHandlerWrapper(int signal) {
    if (g_agent) {
        g_agent->shutdown();
    }
}

Agent::Agent() : running_(false), iterationCount_(0), maxIterations_(0) {
    g_agent = this;
}

Agent::~Agent() {
    shutdown();
    g_agent = nullptr;
}

bool Agent::initialize(const std::string& configPath) {
    auto& logger = Logger::getInstance();
    
    // Load configuration
    if (!configManager_.loadConfig(configPath)) {
        logger.error("Failed to load configuration");
        return false;
    }
    
    auto& agentConfig = configManager_.getAgentConfig();
    
    // Set log level
    if (agentConfig.logLevel == "DEBUG") {
        logger.setLogLevel(LogLevel::DEBUG);
    } else if (agentConfig.logLevel == "INFO") {
        logger.setLogLevel(LogLevel::INFO);
    } else if (agentConfig.logLevel == "WARNING") {
        logger.setLogLevel(LogLevel::WARNING);
    } else if (agentConfig.logLevel == "ERROR") {
        logger.setLogLevel(LogLevel::ERROR);
    }
    
    // Store collection iteration setting
    maxIterations_ = agentConfig.collectionIteration;
    
    // Create metrics aggregator
    aggregator_ = std::make_unique<MetricsAggregator>(agentConfig.maxParallelPlugins);
    
    // Load plugins
    for (const auto& metricConfig : configManager_.getMetrics()) {
        if (!metricConfig.enabled) {
            logger.info("Metric disabled, skipping: ", metricConfig.name);
            continue;
        }
        
        auto plugin = PluginLoader::createPlugin(metricConfig);
        if (plugin) {
            aggregator_->addPlugin(plugin);
        }
    }
    
    // Create exporters
    if (!createExporters()) {
        logger.error("Failed to create exporters");
        return false;
    }
    
    // Setup signal handlers
    setupSignalHandlers();
    
    logger.info("Agent initialized successfully");
    return true;
}

void Agent::run() {
    auto& logger = Logger::getInstance();
    auto& agentConfig = configManager_.getAgentConfig();
    
    running_ = true;
    iterationCount_ = 0;
    
    logger.info("Starting metric collection");
    logger.info("Collection interval: ", agentConfig.collectionIntervalSeconds, " seconds");
    if (maxIterations_ > 0) {
        logger.info("Will run for ", maxIterations_, " iterations");
    } else {
        logger.info("Running indefinitely (press Ctrl+C to stop)");
    }
    
    while (running_) {
        // Check if we've reached the maximum iterations
        if (maxIterations_ > 0 && iterationCount_ >= maxIterations_) {
            logger.info("Reached maximum iterations (", maxIterations_, "), exiting gracefully");
            break;
        }
        
        iterationCount_++;
        logger.info("=== Collection iteration ", iterationCount_, " ===");
        
        // Collect metrics
        auto allMetrics = aggregator_->collectAllMetrics();
        
        // Create aggregated metrics structure
        exporters::AggregatedMetrics aggMetrics;
        aggMetrics.deviceId = agentConfig.deviceId;
        aggMetrics.location = agentConfig.location;
        
        // Get current timestamp
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        aggMetrics.timestamp = oss.str();
        aggMetrics.metricsByPlugin = allMetrics;
        
        // Export metrics
        for (auto& exporter : exporters_) {
            try {
                exporter->exportMetrics(aggMetrics);
            } catch (const std::exception& e) {
                logger.error("Exception in exporter: ", e.what());
            }
        }
        
        // Sleep until next collection
        if (maxIterations_ == 0 || iterationCount_ < maxIterations_) {
            logger.debug("Sleeping for ", agentConfig.collectionIntervalSeconds, " seconds");
            for (int i = 0; i < agentConfig.collectionIntervalSeconds && running_; ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
    
    logger.info("Agent run loop completed");
}

void Agent::shutdown() {
    auto& logger = Logger::getInstance();
    logger.info("Shutting down agent");
    running_ = false;
}

void Agent::setupSignalHandlers() {
    std::signal(SIGINT, signalHandlerWrapper);
    std::signal(SIGTERM, signalHandlerWrapper);
}

bool Agent::createExporters() {
    auto& logger = Logger::getInstance();
    
    for (const auto& exporterConfig : configManager_.getExporters()) {
        if (!exporterConfig.enabled) {
            logger.info("Exporter disabled, skipping: ", exporterConfig.type);
            continue;
        }
        
        std::shared_ptr<exporters::IExporter> exporter;
        
        if (exporterConfig.type == "html_report") {
            exporter = std::make_shared<exporters::HTMLExporter>();
        } else if (exporterConfig.type == "json") {
            exporter = std::make_shared<exporters::JSONExporter>();
        } else if (exporterConfig.type == "csv") {
            exporter = std::make_shared<exporters::CSVExporter>();
        } else {
            logger.warning("Unknown exporter type: ", exporterConfig.type);
            continue;
        }
        
        if (exporter) {
            if (!exporter->initialize(exporterConfig.outputDir)) {
                logger.error("Failed to initialize exporter: ", exporterConfig.type);
                return false;
            }
            exporters_.push_back(exporter);
            logger.info("Exporter created: ", exporterConfig.type);
        }
    }
    
    if (exporters_.empty()) {
        logger.warning("No exporters configured");
    }
    
    return true;
}

} // namespace core
} // namespace perfsight
