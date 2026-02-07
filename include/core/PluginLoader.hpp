#pragma once

#include "plugins/IMetricPlugin.hpp"
#include "core/ConfigManager.hpp"
#include <string>
#include <memory>

namespace perfsight {
namespace core {

/**
 * @brief Factory for creating plugin instances
 */
class PluginLoader {
public:
    /**
     * @brief Create a plugin instance based on type
     * @param config Metric configuration
     * @return Shared pointer to created plugin, or nullptr if creation failed
     */
    static plugins::PluginPtr createPlugin(const MetricConfig& config);

private:
    PluginLoader() = default;
};

} // namespace core
} // namespace perfsight
