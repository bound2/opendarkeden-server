# DarkEden Makefile

.PHONY: all fmt fmt fmt-check fmt-check-all clean help debug test \
        dev-test dev-build dev-shell dev-clean

# Default target
all: debug

# Wire-contract test suite (docs/RESTRUCTURING.md Phase 1). Runs locally by
# design — there is no CI tier for it yet (Phase 6).
test:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug -DDARKEDEN_BUILD_TESTS=ON
	cmake --build build --target wire_tests -j$(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
	cd build && ctest --output-on-failure

# Containerised build and test (tools/devbuild.sh). Prefer these on a machine
# where the checkout is a bind mount into the container - a Windows host, say -
# because compiling straight off the mount is I/O bound and leaves most of the
# CPU idle. Measured on this project: a full build went from ~20 minutes at
# ~20% CPU to ~3.5 minutes at ~95%, and a no-op rebuild from minutes to
# seconds. The script explains why; it needs the darkeden-dev image
# (docker build -f Dockerfile.dev -t darkeden-dev .).
dev-test:
	bash tools/devbuild.sh test
dev-build:
	bash tools/devbuild.sh build
dev-shell:
	bash tools/devbuild.sh shell
dev-clean:
	bash tools/devbuild.sh clean

release:
	cmake -B build -DCMAKE_BUILD_TYPE=Release -DDARKEDEN_BUILD_TESTS=OFF
	cmake --build build -j$(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug -DDARKEDEN_BUILD_TESTS=OFF
	cmake --build build -j$(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)


# Format code with clang-format
fmt:
	@echo "Formatting C++ code with clang-format..."
	find src tests -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i {} +
	@echo "Code formatted successfully!"

# Check format for modified files only (fast)
fmt-check:
	@echo "Checking format for modified files..."
	@failed=false; \
	files=$$(git diff --name-only --diff-filter=ACM | grep -E '\.(cpp|h|hpp)$$' || true); \
	if [ -z "$$files" ]; then \
		echo "No modified C++ files to check."; \
	else \
		for file in $$files; do \
			if [ -f "$$file" ]; then \
				if ! clang-format "$$file" | diff -q "$$file" - > /dev/null 2>&1; then \
					echo "[FAIL] $$file needs formatting"; \
					failed=true; \
				fi; \
			fi; \
		done; \
	fi; \
	if $$failed; then \
		echo ""; \
		echo "Run 'make fmt' to fix formatting issues"; \
		exit 1; \
	fi; \
	echo "[OK] All modified files are properly formatted!"

# Check format for all files (slow)
fmt-check-all:
	@echo "Checking format for ALL files..."
	@failed=false; \
	for file in $$(find src tests -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \)); do \
		if ! clang-format "$$file" | diff -q "$$file" - > /dev/null 2>&1; then \
			echo "[FAIL] $$file needs formatting"; \
			failed=true; \
		fi; \
	done; \
	if $$failed; then \
		echo ""; \
		echo "Run 'make fmt' to fix formatting issues"; \
		exit 1; \
	fi; \
	echo "[OK] All files are properly formatted!"

# Clean build artifacts
clean:
	rm -rf build bin lib

# Show help message
help:
	@echo "DarkEden Makefile targets:"
	@echo "  all           - Build the project (default)"
	@echo "  fmt           - Format all C++ code"
	@echo "  fmt-check     - Check format for modified files only (fast)"
	@echo "  test          - Build and run the wire-contract test suite"
	@echo "  fmt-check-all - Check format for all files (slow)"
	@echo "  clean         - Clean build artifacts"
	@echo "  help          - Show this help message"
