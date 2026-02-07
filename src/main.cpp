#include "core/Agent.hpp"
#include "core/Logger.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

void printUsage(const char* programName) {
    std::cout << "perfSight - Lightweight Performance Monitoring Tool\n\n";
    std::cout << "Usage: " << programName << " --config <config_file>\n\n";
    std::cout << "Options:\n";
    std::cout << "  --config <file>    Path to YAML configuration file (required)\n";
    std::cout << "  --help             Show this help message\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << programName << " --config perfsight.conf.yaml\n";
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string configPath;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--config" && i + 1 < argc) {
            configPath = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    if (configPath.empty()) {
        std::cerr << "Error: Configuration file not specified\n\n";
        printUsage(argv[0]);
        return 1;
    }
    
    // Initialize logger
    auto& logger = perfsight::core::Logger::getInstance();
    logger.init("/tmp/perfsight/logs/perfsight.log");
    
    logger.info("=================================================");
    logger.info("       perfSight - Performance Monitor");
    logger.info("=================================================");
    
    // Create and initialize agent
    perfsight::core::Agent agent;
    
    if (!agent.initialize(configPath)) {
        logger.error("Failed to initialize agent");
        return 1;
    }
    
    // Run agent
    try {
        agent.run();
    } catch (const std::exception& e) {
        logger.error("Agent crashed with exception: ", e.what());
        return 1;
    }
    
    logger.info("perfSight agent terminated");
    return 0;
}
