cmake_minimum_required(VERSION 3.12)
project(perfSight VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# Find threads library (required for std::thread)
find_package(Threads REQUIRED)

# Source files - Core
set(CORE_SOURCES
    src/core/ConfigManager.cpp
    src/core/PluginLoader.cpp
    src/core/MetricsAggregator.cpp
    src/core/Agent.cpp
)

# Source files - Plugins
set(PLUGIN_SOURCES
    src/plugins/ProcMetricsPlugin.cpp
    src/plugins/SoCMetricsPlugin.cpp
    src/plugins/ContainerMetricsPlugin.cpp
)

# Source files - Exporters
set(EXPORTER_SOURCES
    src/exporters/HTMLExporter.cpp
    src/exporters/JSONExporter.cpp
    src/exporters/CSVExporter.cpp
)

# Main executable
add_executable(perfsight
    src/main.cpp
    ${CORE_SOURCES}
    ${PLUGIN_SOURCES}
    ${EXPORTER_SOURCES}
)

# Link libraries
target_link_libraries(perfsight
    Threads::Threads
    stdc++fs  # For std::filesystem support
)

# Installation rules
install(TARGETS perfsight
    RUNTIME DESTINATION bin
)

install(FILES perfsight.conf.yaml
    DESTINATION /etc/perfsight
)

# Create log directory
install(DIRECTORY DESTINATION /var/log/perfsight)

# Print configuration info
message(STATUS "perfSight Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
