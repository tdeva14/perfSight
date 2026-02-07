# perfSight 🔍

**Lightweight, Modular Performance Monitoring Tool for Embedded Systems**

perfSight is a plugin-based performance monitoring solution designed for embedded devices and resource-constrained systems. It provides real-time metric collection with support for multiple output formats and future cloud integration capabilities.

## Features

- ⚡ **Lightweight & Efficient**: Minimal resource footprint suitable for embedded systems
- 🔌 **Plugin Architecture**: Extensible metric collectors for various subsystems
- 📊 **Multiple Output Formats**: HTML reports, JSON, and CSV for easy integration
- ⚙️ **Configurable**: YAML-based configuration for flexible deployment
- 🛡️ **Fault Tolerant**: Plugin isolation and auto-restart on failure
- 🔄 **Real-time Monitoring**: Configurable collection intervals
- 🚀 **Cloud-Ready**: Designed for easy integration with Prometheus, MQTT, REST APIs

## Architecture

perfSight follows a modular, plugin-based architecture:

```
┌─────────────────────────────────────────────────────-┐
│                  perfSight Agent                     │
├──────────────────────────────────────────────────────┤
│  Config Manager  │  Plugin Loader   │  Scheduler     │
├──────────────────┴──────────────────┴────────────────┤
│                Metrics Aggregator                    │
├──────────────────────────────────────────────────────┤
│   Metric Plugins          │    Output Exporters      │
│   • SystemMetrics         │    • HTML Reporter       │
│   • ProcMetrics           │    • JSON Exporter       │
│   • ContainerMetrics      │    • CSV Exporter        │
│   • Custom Plugins...     │    • Custom Exporters... │
└──────────────────────────────────────────────────────┘
```

### Core Components

1. **Agent**: Main orchestrator handling lifecycle and scheduling
2. **Config Manager**: YAML configuration parser and validator
3. **Plugin Loader**: Dynamic plugin registration and management
4. **Metrics Aggregator**: Parallel metric collection with controlled concurrency
5. **Logger**: Thread-safe logging with configurable levels

## Quick Start

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.12 or higher
- Linux system (tested on Ubuntu, Debian, embedded Linux)
- pthread support

### Build

```bash
# Clone the repository
cd perfSight

# Build using Makefile (wraps CMake)
make build

# Or build with debug symbols
make debug

# Run the tool
./build/perfsight --config perfsight.conf.yaml
```

### Installation

```bash
# Install to /usr/local (default)
sudo make install

# Or specify custom installation prefix
sudo make install PREFIX=/opt/perfsight

# Run installed version
perfsight --config /etc/perfsight/perfsight.conf.yaml
```

## Configuration

perfSight uses a YAML configuration file for all settings. See `perfsight.conf.yaml` for a complete example.

### Agent Configuration

```yaml
agent:
  device_id: "device-1234"              # Unique device identifier
  location: "rack1A"                    # Physical location
  log_level: "INFO"                     # DEBUG, INFO, WARNING, ERROR
  collection_interval_seconds: 30       # How often to collect metrics
  collection_iteration: 0               # Number of iterations (0 = run indefinitely)
  max_parallel_plugins: 4               # Max concurrent plugin execution
  reliability:
    restart_plugins_on_failure: true    # Auto-restart failed plugins
    plugin_isolation: true              # Isolate plugin failures
```

### Metric Plugins

#### System Metrics Plugin

Collects system-wide metrics from `/proc` filesystem:

```yaml
metrics:
  - name: system_metrics
    enabled: true
    plugin: system
    params: {}
```

**Collected Metrics:**
- **Memory** (from `/proc/meminfo`): Total, Free, Available, Buffers, Cached, Swap
- **CPU** (from `/proc/stat`): User, System, Nice, Idle, I/O wait, IRQ, Soft IRQ times
- **Memory Fragmentation** (from `/proc/buddyinfo`): Free pages per order for each zone

#### Process Metrics Plugin

Collects per-process metrics from `/proc/[pid]/` filesystem:

```yaml
metrics:
  - name: process_metrics
    enabled: true
    plugin: proc
    params:
      process_whitelist: ["nginx", "mysql"]  # Monitor specific processes
      include_kernel_threads: false
```

**Collected Metrics** (per process):
- **Process Stats** (from `/proc/[pid]/stat`): State, CPU times (user/system), thread count, virtual/resident memory
- **Process Status** (from `/proc/[pid]/status`): VmRSS, VmSize, VmPeak, VmHWM
- **Memory Maps** (from `/proc/[pid]/smaps`): PSS (Proportional Set Size), Private/Shared Clean/Dirty memory

#### Container Metrics Plugin

Collects container metrics via cgroup interface:

```yaml
metrics:
  - name: container_memory
    enabled: true
    plugin: container
    params: {}
```

**Collected Metrics:**
- Container memory usage (cgroup v1 & v2)
- Container memory limits
- Auto-detection of running containers

### Output Exporters

```yaml
exporters:
  - type: html_report
    enabled: true
    output_dir: "/var/log/perfsight"
    
  - type: json
    enabled: true
    output_dir: "/var/log/perfsight/json"
    
  - type: csv
    enabled: true
    output_dir: "/var/log/perfsight/csv"
```

### Future Extensibility

perfSight is designed to easily integrate future features:

```yaml
# Not yet implemented, but infrastructure ready
prometheus:
  enabled: false
  port: 9090

api:
  enabled: false
  port: 8080
  
healthcheck:
  enabled: false
  endpoint: "/health"
```

## Metrics Reference

### System Memory Metrics

| Metric | Unit | Description |
|--------|------|-------------|
| MemTotal | kB | Total system RAM |
| MemFree | kB | Unused RAM |
| MemAvailable | kB | Estimated available memory |
| Buffers | kB | Temporary buffer cache |
| Cached | kB | Page cache |
| SwapTotal | kB | Total swap space |
| SwapFree | kB | Available swap |
| SwapCached | kB | Swap cached |
| Active | kB | Active memory |
| Inactive | kB | Inactive memory |

### CPU Metrics

| Metric | Unit | Description |
|--------|------|-------------|
| cpu_user | jiffies | Time in user mode |
| cpu_nice | jiffies | Time in user mode with low priority |
| cpu_system | jiffies | Time in kernel mode |
| cpu_idle | jiffies | Idle time |
| cpu_iowait | jiffies | Waiting for I/O |
| cpu_irq | jiffies | Servicing interrupts |
| cpu_softirq | jiffies | Servicing soft interrupts |

### Memory Fragmentation Metrics

| Metric | Unit | Description |
|--------|------|-------------|
| buddyinfo_{zone}_orders | - | Free pages per order for memory zone |

### Process Metrics (Per Process)

| Metric | Unit | Description |
|--------|------|-------------|
| {process}_{pid}_state | - | Process state (R/S/D/Z/T) |
| {process}_{pid}_utime | jiffies | User CPU time |
| {process}_{pid}_stime | jiffies | System CPU time |
| {process}_{pid}_num_threads | - | Number of threads |
| {process}_{pid}_vsize | bytes | Virtual memory size |
| {process}_{pid}_rss | pages | Resident set size |
| {process}_{pid}_VmRSS | kB | Resident memory |
| {process}_{pid}_VmSize | kB | Virtual memory size |
| {process}_{pid}_VmPeak | kB | Peak virtual memory |
| {process}_{pid}_VmHWM | kB | Peak resident memory |
| {process}_{pid}_Pss | kB | Proportional set size |
| {process}_{pid}_Private_Clean | kB | Private clean memory |
| {process}_{pid}_Private_Dirty | kB | Private dirty memory |
| {process}_{pid}_Shared_Clean | kB | Shared clean memory |
| {process}_{pid}_Shared_Dirty | kB | Shared dirty memory |

## Output Examples

### HTML Report

perfSight generates a clean, responsive HTML report:

```
/var/log/perfsight/perfsight_report.html
```

Features:
- Responsive design for viewing on any device
- Organized by plugin/metric category
- Sortable tables
- Device metadata (ID, location, timestamp)

### JSON Output

Machine-readable JSON format for integration:

```json
{
  "device_id": "device-1234",
  "location": "rack1A",
  "timestamp": "2026-02-07 10:30:00",
  "metrics": {
    "system_metrics": [
      {
        "name": "MemTotal",
        "value": "8192000",
        "unit": "kB",
        "description": "Total system memory"
      },
      {
        "name": "cpu_user",
        "value": "12345",
        "unit": "jiffies",
        "description": "User mode CPU time"
      }
    ],
    "process_metrics": [
      {
        "name": "nginx_1234_VmRSS",
        "value": "45678",
        "unit": "kB",
        "description": "Resident memory"
      }
    ]
  }
}
```

### CSV Export

Flat CSV format for spreadsheet analysis:

```csv
Device ID,Location,Timestamp,Plugin,Metric Name,Value,Unit,Description
device-1234,rack1A,2026-02-07 10:30:00,system_metrics,MemTotal,8192000,kB,Total system memory
device-1234,rack1A,2026-02-07 10:30:00,process_metrics,nginx_1234_VmRSS,45678,kB,Resident memory
```

## Extending perfSight

### Adding a Custom Plugin

1. **Create plugin header** in `include/plugins/`:

```cpp
#pragma once
#include "IMetricPlugin.hpp"

class MyCustomPlugin : public IMetricPlugin {
public:
    bool initialize(const PluginParams& params) override;
    MetricData collectMetrics() override;
    std::string getName() const override;
    void shutdown() override;
    bool isHealthy() const override;
};
```

2. **Implement plugin** in `src/plugins/`:

```cpp
#include "plugins/MyCustomPlugin.hpp"

MetricData MyCustomPlugin::collectMetrics() {
    MetricData metrics;
    // Your collection logic here
    metrics.push_back(MetricValue("my_metric", "123", "units", "Description"));
    return metrics;
}
```

3. **Register in PluginLoader**:

```cpp
// In src/core/PluginLoader.cpp
if (pluginType == "mycustom") {
    return std::make_shared<MyCustomPlugin>(metricName);
}
```

4. **Update configuration**:

```yaml
metrics:
  - name: my_custom_metric
    enabled: true
    plugin: mycustom
    params: {}
```

### Adding a Custom Exporter

Similar process - implement `IExporter` interface and register in Agent.

## Development

### Project Structure

```
perfSight/
├── CMakeLists.txt          # Build configuration
├── Makefile                # Convenience wrapper
├── perfsight.conf.yaml     # Sample configuration
├── README.md               # This file
├── include/                # Header files
│   ├── core/              # Core components
│   ├── plugins/           # Metric plugins
│   └── exporters/         # Output exporters
├── src/                    # Implementation files
│   ├── main.cpp           # Entry point
│   ├── core/              # Core implementations
│   ├── plugins/           # Plugin implementations
│   └── exporters/         # Exporter implementations
└── examples/               # Example outputs
```

### Code Style

- C++17 standard
- Header guards using `#pragma once`
- Namespace: `perfsight::{core,plugins,exporters}`
- Smart pointers for resource management
- RAII principles
- Comprehensive error handling

### Building for Embedded Systems

For cross-compilation:

```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release ..
make
```

Optimize for size:

```bash
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
make
strip perfsight  # Remove debug symbols
```

## Troubleshooting

### Common Issues

**Permission Denied:**
- Ensure output directories are writable
- Some metrics require root access (e.g., `/sys/kernel/debug`)

**Plugin Not Loading:**
- Check plugin is enabled in configuration
- Verify plugin parameters are correct
- Check logs for initialization errors

**No Metrics Collected:**
- Verify `/proc` and `/sys` are mounted
- Check file permissions
- Ensure kernel exposes required interfaces

### Debug Mode

Enable debug logging:

```yaml
agent:
  log_level: "DEBUG"
```

Check logs:

```bash
tail -f /var/log/perfsight/perfsight.log
```

## Performance

perfSight is optimized for embedded systems:

- **Memory**: < 10 MB RSS typical
- **CPU**: < 1% average (with 30s interval)
- **Disk**: Minimal (rotated logs, periodic reports)
- **Startup**: < 500ms

## License

MIT License - See LICENSE file for details

## Contributing

Contributions welcome! Areas of interest:

- Additional SoC support
- New metric plugins (Network, Thermal, Power)
- Cloud integrations (Prometheus, InfluxDB, MQTT)
- Performance optimizations
- Documentation improvements

## Roadmap

- [ ] Prometheus exporter
- [ ] REST API for real-time queries
- [ ] Web dashboard
- [ ] Alert/threshold monitoring
- [ ] Historical data storage
- [ ] MQTT publisher
- [ ] Time-series database integration

## Support

For issues, questions, or contributions:
- GitHub Issues: Report bugs and feature requests
- Documentation: See `/docs` directory
- Examples: See `/examples` directory

---
