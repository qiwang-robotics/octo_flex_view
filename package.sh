#!/bin/bash

# OctoFlexView Packaging Script
# Packages the library, headers, CMake config files, and examples for distribution

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

# Default values
PACKAGE_NAME="octo_flex_view"
BUILD_TYPE="Release"
INSTALL_PREFIX="/tmp/octoflexview-package"
PACKAGE_FORMAT="tar.gz"
BUILD_DIR="build"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $F in
        -n|--name)
            PACKAGE_NAME="$2"
            shift 2
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        -f|--format)
            PACKAGE_FORMAT="$2"
            shift 2
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -n, --name NAME          Package name (default: octo_flex_view)"
            echo "  -t, --type BUILD_TYPE    Build type (default: Release)"
            echo "  -p, --prefix PREFIX      Install prefix (default: /tmp/octoflexview-package)"
            echo "  -f, --format FORMAT      Package format: tar.gz, zip (default: tar.gz)"
            echo "  -b, --build-dir DIR      Build directory (default: build)"
            echo "  -h, --help               Show this help message"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check dependencies.
check_dependencies() {
    log_info "Checking system dependencies..."

    local missing_deps=()

    # Check required tools.
    command -v cmake >/dev/null 2>&1 || missing_deps+=("cmake")
    command -v make >/dev/null 2>&1 || missing_deps+=("make")
    command -v g++ >/dev/null 2>&1 || missing_deps+=("g++")
    command -v tar >/dev/null 2>&1 || missing_deps+=("tar")
    command -v git >/dev/null 2>&1 || missing_deps+=("git")

    if [ ${#missing_deps[@]} -ne 0 ]; then
        log_error "Missing dependencies: ${missing_deps[*]}"
        log_info "Run the following command to install dependencies:"
        log_info "sudo apt-get install cmake make g++ tar git"
        exit 1
    fi

    log_success "All dependency checks passed"
}

# Clean old build and package directories
clean_build() {
    log_info "Cleaning old build and package directories..."

    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        log_info "Removed $BUILD_DIR directory"
    fi

    if [ -d "$INSTALL_PREFIX" ]; then
        rm -rf "$INSTALL_PREFIX"
        log_info "Removed install prefix directory: $INSTALL_PREFIX"
    fi

    # Clean up any previous packages
    rm -f "${PACKAGE_NAME}_"*".${PACKAGE_FORMAT}"* 2>/dev/null || true

    log_success "Build and package directories cleaned"
}

# Build the library
build_library() {
    log_info "Building OctoFlexView library..."

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    log_info "Running CMake configuration..."
    cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"

    log_info "Compiling library..."
    make -j$(nproc)

    cd ..

    # Check whether the library file was produced
    if [ -f "$BUILD_DIR/libocto_flex_view.so" ]; then
        log_success "Library build complete: $BUILD_DIR/libocto_flex_view.so"
    else
        log_error "Library build failed"
        exit 1
    fi
}

# Install the library to the package directory
install_library() {
    log_info "Installing library to package directory: $INSTALL_PREFIX"

    cd "$BUILD_DIR"
    
    # Use CMake to install the library and headers
    make install
    
    cd ..
    
    log_success "Library installed to $INSTALL_PREFIX"
}

# Package headers only
package_headers() {
    log_info "Packaging interface header files..."

    # Create the include directory in the install prefix
    mkdir -p "$INSTALL_PREFIX/include/octo_flex"

    # Copy only the interface header files from the include directory
    cp include/*.h "$INSTALL_PREFIX/include/octo_flex/"

    # Copy documentation files from include directory
    cp include/*.md "$INSTALL_PREFIX/include/octo_flex/" 2>/dev/null || true

    log_success "Interface header files and documentation copied to package"
}

# Package documentation
package_documentation() {
    log_info "Packaging documentation..."

    # Copy documentation files
    cp README.md "$INSTALL_PREFIX/" 2>/dev/null || true
    cp README_CN.md "$INSTALL_PREFIX/" 2>/dev/null || true
    cp LICENSE "$INSTALL_PREFIX/" 2>/dev/null || true
    cp THIRD_PARTY_NOTICES "$INSTALL_PREFIX/" 2>/dev/null || true
    
    # Create a package info file
    cat > "$INSTALL_PREFIX/PACKAGE_INFO" << EOF
OctoFlexView Library Package
Version: $(grep "project(octo_flex_view" CMakeLists.txt | grep -o "[0-9].[0-9].[0-9]")
Build Type: $BUILD_TYPE
Build Date: $(date)
Package Format: $PACKAGE_FORMAT
Package Name: $PACKAGE_NAME
EOF

    log_success "Documentation packaged"
}

# Package examples
package_examples() {
    log_info "Packaging examples..."

    # Create examples directory in the install prefix
    mkdir -p "$INSTALL_PREFIX/examples"

    # Copy all example directories except build directories
    if [ -d "examples" ]; then
        for dir in examples/*/; do
            dir_name=$(basename "$dir")
            if [ "$dir_name" != "build" ]; then
                cp -r "$dir" "$INSTALL_PREFIX/examples/"
                # Remove build directory if it exists in the copied example
                rm -rf "$INSTALL_PREFIX/examples/$dir_name/build" 2>/dev/null || true
            fi
        done
        log_success "Examples packaged (build directories excluded)"
    else
        log_warning "Examples directory not found, skipping..."
    fi
}

# Create the final package archive
create_package() {
    log_info "Creating package archive..."

    # Store the original working directory
    ORIGINAL_DIR=$(pwd)

    # Create the archive in the current directory
    cd "$(dirname "$INSTALL_PREFIX")"

    PACKAGE_FILENAME="${PACKAGE_NAME}_$(date +%Y%m%d_%H%M%S).${PACKAGE_FORMAT}"

    case "$PACKAGE_FORMAT" in
        "tar.gz")
            tar -czf "$PACKAGE_FILENAME" "$(basename "$INSTALL_PREFIX")"
            ;;
        "zip")
            zip -r "$PACKAGE_FILENAME" "$(basename "$INSTALL_PREFIX")"
            ;;
        *)
            log_error "Unsupported package format: $PACKAGE_FORMAT"
            exit 1
            ;;
    esac

    # Move the package to the original working directory
    mv "$PACKAGE_FILENAME" "$ORIGINAL_DIR/$PACKAGE_FILENAME"

    cd "$ORIGINAL_DIR"

    log_success "Package created: $ORIGINAL_DIR/$PACKAGE_FILENAME (Size: $(ls -lh "$ORIGINAL_DIR/$PACKAGE_FILENAME" | awk '{print $5}'))"
}

# Generate package report
generate_package_report() {
    log_info "Generating package report..."

    local report_file="package_report.txt"

    cat > "$report_file" << EOF
OctoFlexView Package Report
Generated: $(date)
Package Name: $PACKAGE_NAME
Build Type: $BUILD_TYPE
Package Format: $PACKAGE_FORMAT
Install Prefix: $INSTALL_PREFIX

=== Package Contents ===
Library:
$(find "$INSTALL_PREFIX" -name "libocto_flex_view.so" -exec ls -la {} \; 2>/dev/null || echo "  Library not found")

Headers:
$(find "$INSTALL_PREFIX/include" -name "*.h" -exec basename {} \; 2>/dev/null | sed 's/^/  /' || echo "  Headers not found")

CMake Config Files:
$(find "$INSTALL_PREFIX/lib/cmake" -name "*.cmake" -exec basename {} \; 2>/dev/null | sed 's/^/  /' || echo "  CMake config files not found")

Examples:
$(find "$INSTALL_PREFIX/examples" -mindepth 1 -maxdepth 1 -type d -exec basename {} \; 2>/dev/null | sed 's/^/  /' || echo "  Examples not found")

=== Installation Instructions ===
1. Extract the package to your desired location
2. Set CMAKE_PREFIX_PATH to the package directory when building your project
3. Use find_package(OctoFlexView REQUIRED) in your CMakeLists.txt
4. Link against OctoFlexView::octo_flex_view

=== Usage Example ===
#include \"octo_flex_viewer.h\"
using namespace octo_flex;

int main(int argc, char* argv[]) {
    auto viewer = OctoFlexViewer::create(\"Hello OctoFlexView\");
    viewer.addSphere(\"sphere\", Vec3(1, 0, 0), 1.0);
    viewer.show();
    return viewer.run();
}
EOF

    log_success "Package report generated: $report_file"
}

# Main function
main() {
    log_info "=== OctoFlexView Packaging Start ==="
    log_info "Package Name: $PACKAGE_NAME"
    log_info "Build Type: $BUILD_TYPE"
    log_info "Install Prefix: $INSTALL_PREFIX"
    log_info "Package Format: $PACKAGE_FORMAT"
    log_info "Build Directory: $BUILD_DIR"

    # Record start time
    local start_time=$(date +%s)

    # Execute packaging steps
    check_dependencies
    clean_build
    build_library
    install_library
    package_headers
    package_documentation
    package_examples
    create_package
    generate_package_report

    # Calculate packaging time
    local end_time=$(date +%s)
    local package_time=$((end_time - start_time))

    log_success "=== Packaging Complete ==="
    log_info "Total time: ${package_time}s"
    log_info "Package file: $(ls -1 ${PACKAGE_NAME}_*.${PACKAGE_FORMAT} | head -n1)"
    log_info "Package report: package_report.txt"
}

# Run main function.
main "$@"