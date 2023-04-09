CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic

RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
NC = \033[0m

BUILD_DIR = build

all: renderer

renderer: $(BUILD_DIR)
	@echo "$(YELLOW)Generating build files...$(NC)"
	cd $(BUILD_DIR) && cmake ..
	@if [ $$? -ne 0 ]; then \
		echo "$(RED)Error: CMake failed to generate build files.$(NC)"; \
		exit 1; \
	fi
	@echo "$(YELLOW)Building project...$(NC)"
	cmake --build $(BUILD_DIR)
	@if [ $$? -ne 0 ]; then \
		echo "$(RED)Error: Build failed.$(NC)"; \
		exit 1; \
	fi
	@echo "$(GREEN)---------------------------"; \
	echo "Build complete. Running renderer"; \
	echo "---------------------------$(NC)"
	@cd .. && ./renderer.exe

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all clean
