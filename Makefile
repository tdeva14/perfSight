# perfSight Makefile - Wrapper for CMake build

# Default build type
BUILD_TYPE ?= Release

# Build directory
BUILD_DIR = build

# Installation prefix
PREFIX ?= /usr/local

.PHONY: all build clean install uninstall run help

all: build

# Configure and build using CMake
build:
	@echo "Building perfSight..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_INSTALL_PREFIX=$(PREFIX) ..
	@cd $(BUILD_DIR) && $(MAKE)
	@echo "Build complete! Binary: $(BUILD_DIR)/perfsight"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

# Install (requires sudo for system directories)
install: build
	@echo "Installing perfSight..."
	@cd $(BUILD_DIR) && sudo $(MAKE) install
	@sudo mkdir -p /var/log/perfsight
	@sudo mkdir -p /tmp/perfsight
	@echo "Installation complete!"

# Uninstall
uninstall:
	@echo "Uninstalling perfSight..."
	@sudo rm -f $(PREFIX)/bin/perfsight
	@sudo rm -rf /etc/perfsight
	@echo "Uninstall complete!"

# Run perfSight with default config
run: build
	@echo "Running perfSight..."
	@mkdir -p /tmp/perfsight
	@$(BUILD_DIR)/perfsight --config perfsight.conf.yaml

# Run in debug mode
debug:
	@$(MAKE) BUILD_TYPE=Debug build
	@echo "Debug build complete!"

# Help
help:
	@echo "perfSight Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make build        - Build perfSight (default)"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make install      - Install perfSight to system (requires sudo)"
	@echo "  make uninstall    - Remove perfSight from system"
	@echo "  make run          - Build and run with default config"
	@echo "  make debug        - Build with debug symbols"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_TYPE        - Release (default) or Debug"
	@echo "  PREFIX            - Installation prefix (default: /usr/local)"
	@echo ""
	@echo "Examples:"
	@echo "  make build BUILD_TYPE=Debug"
	@echo "  make install PREFIX=/opt/perfsight"
