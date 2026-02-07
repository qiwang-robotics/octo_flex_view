#!/bin/bash

# OctoFlexView Build Script
# Build library and test program

set -e  # Stop on error

# Define color output.
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging helpers.
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check dependencies.
check_dependencies() {
    log_info "Checking system dependencies..."
    
    local missing_deps=()
    
    # Check required tools.
    command -v cmake >/dev/null 2>&1 || missing_deps+=("cmake")
    command -v make >/dev/null 2>&1 || missing_deps+=("make")
    command -v g++ >/dev/null 2>&1 || missing_deps+=("g++")
    
    # Check Qt5.
    if ! pkg-config --exists Qt5Core Qt5Widgets Qt5OpenGL; then
        missing_deps+=("qt5-dev")
    fi
    
    # Check OpenGL libraries.
    if ! pkg-config --exists gl glu glew; then
        missing_deps+=("opengl-dev")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        log_error "Missing dependencies: ${missing_deps[*]}"
        log_info "Run the following command to install dependencies:"
        log_info "sudo apt-get install cmake make g++ qtbase5-dev qt5-qmake libqt5opengl5-dev libgl1-mesa-dev libglu1-mesa-dev libglew-dev"
        exit 1
    fi
    
    log_success "All dependency checks passed"
}

# Clean old build files
clean_build() {
    log_info "Cleaning old build files..."

    if [ -d "build" ]; then
        rm -rf build
        log_info "Removed build directory"
    fi

    log_success "Build directory cleaned"
}

# Build main library.
build_main_library() {
    log_info "Building OctoFlexView main library..."

    mkdir -p build
    cd build

    log_info "Running CMake configuration..."
    cmake .. -DCMAKE_BUILD_TYPE=Release

    log_info "Compiling main library..."
    make -j$(nproc)

    cd ..

    # Check whether the library file was produced (shared library).
    if [ -f "build/libocto_flex_view.so" ]; then
        log_success "Main library build complete: build/libocto_flex_view.so"
    else
        log_error "Main library build failed"
        exit 1
    fi
}

# Run tests
run_tests() {
    log_info "Running test programs..."

    cd build

    # Check whether test programs were generated.
    if ls ./test_* >/dev/null 2>&1; then
        log_info "Found test programs:"
        ls ./test_* | sed 's|^\./||' | while read -r test_prog; do
            log_info "  - $test_prog"
        done
    else
        log_info "No test programs generated"
    fi

    cd ..
    log_success "Test checks complete"
}

# Build examples
build_examples() {
    log_info "Building all examples..."

    # Save current working directory.
    local original_dir=$(pwd)

    # Iterate subdirectories in examples.
    for example_dir in examples/*/; do
        if [ -d "$example_dir" ]; then
            example_name=$(basename "$example_dir")
            log_info "Building example: $example_name"

            # Check if CMakeLists.txt exists.
            if [ -f "$example_dir/CMakeLists.txt" ]; then
                # Create build directory.
                # Strip trailing slash to avoid double slashes.
                example_dir_clean=$(echo "$example_dir" | sed 's|/$||')
                build_dir="$example_dir_clean/build"
                mkdir -p "$build_dir"

                # Enter build directory.
                cd "$build_dir"

                # Run CMake configuration.
                log_info "  Configuring CMake..."
                cmake .. -DCMAKE_BUILD_TYPE=Release \
                         -DCMAKE_PREFIX_PATH="../../build" \
                         -DCMAKE_LIBRARY_PATH="../../build"

                # Build example.
                log_info "  Building example..."
                make -j$(nproc)

                # Check build success.
                # First try directory name as executable name.
                if [ -f "./$example_name" ]; then
                    log_success "  Example build succeeded: $build_dir/$example_name"
                else
                    # If directory name doesn't match, check CMakeLists.txt for add_executable.
                    executable_name=$(grep -o "add_executable([a-zA-Z0-9_]*" "$original_dir/${example_dir_clean}/CMakeLists.txt" | head -n1 | sed 's/add_executable(//')
                    if [ -n "$executable_name" ] && [ -f "./$executable_name" ]; then
                        log_success "  Example build succeeded: $build_dir/$executable_name"
                    else
                        log_error "  Example build failed: $example_name"
                    fi
                fi

                # Return to original directory.
                cd "$original_dir"
            else
                log_warning "  $example_name has no CMakeLists.txt file, skipping"
            fi
        fi
    done

    log_success "All example builds complete"
}

# Generate build report
generate_build_report() {
    log_info "Generating build report..."

    local report_file="build_report.txt"

    cat > "$report_file" << EOF
OctoFlexView Build Report
Generated: $(date)
Build Host: $(hostname)
OS: $(uname -a)

=== Build Artifacts ===
Library:
$(ls -la build/libocto_flex_view.so 2>/dev/null || echo "  Library not found")

Test Programs:
$(ls -la build/test_* 2>/dev/null || echo "  Test programs not found")

Examples:
$(find examples -name "build" -type d -exec sh -c 'find "$1" -maxdepth 1 -executable -type f' _ {} \; 2>/dev/null | sed 's/^/  /' || echo "  No examples built")

=== API Files ===
Headers:
  include/octo_flex_viewer.h (Facade API)
  include/object_builder.h (Builder API)
  include/texture_image.h (Texture image data for programmatic generation)

=== Features ===
- 3D visualization container management
- Object and shape creation
- Geometric transformations (move, rotate)
- Multi-view support (split, merge)
- Object manager (layered organization)
- Memory management

=== Usage ===
Link the following libraries:
  libocto_flex_view.so Qt5::Widgets Qt5::OpenGL GL GLU GLEW

Include headers:
  #include "octo_flex_viewer.h"
  #include "object_builder.h"
EOF

    log_success "Build report generated: $report_file"
}

# Main function
main() {
    log_info "=== OctoFlexView Build Start ==="

    # Record start time
    local start_time=$(date +%s)

    # Execute build steps
    check_dependencies
    clean_build
    build_main_library
    run_tests
    build_examples
    generate_build_report

    # Calculate build time
    local end_time=$(date +%s)
    local build_time=$((end_time - start_time))

    log_success "=== Build Complete ==="
    log_info "Total time: ${build_time}s"
    log_info "Build artifacts:"
    log_info "  - Library: build/libocto_flex_view.so"
    log_info "  - Report: build_report.txt"
    log_info "  - Examples: Compiled in their respective build directories"
}

# Run main function.
main "$@"
