#!/usr/bin/env bash
# Build script for Wormhole Qt
# Requirements: Qt6, cmake, ninja (or make), magic-wormhole (pip install magic-wormhole)

set -e

echo "=== Wormhole Qt — Build Script ==="

# Check dependencies
check_dep() {
    if ! command -v "$1" &>/dev/null; then
        echo "[ERROR] '$1' not found. $2"
        exit 1
    fi
}

check_dep cmake  "Install cmake: sudo apt install cmake  OR  sudo dnf install cmake"
check_dep wormhole "Install magic-wormhole: pip install magic-wormhole"

# Detect Qt6
QT6_FOUND=0
for qmake in qmake6 qmake qt6-qmake; do
    if command -v "$qmake" &>/dev/null; then
        QT6_FOUND=1
        break
    fi
done

if [ $QT6_FOUND -eq 0 ]; then
    echo "[ERROR] Qt6 not found."
    echo "  Ubuntu/Debian: sudo apt install qt6-base-dev"
    echo "  Fedora:        sudo dnf install qt6-qtbase-devel"
    echo "  Arch:          sudo pacman -S qt6-base"
    exit 1
fi

BUILD_DIR="build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "--- Configuring..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${PREFIX:-/usr/local}"

echo "--- Building..."
cmake --build . --parallel "$(nproc)"

echo ""
echo "=== Build complete! ==="
echo "Binary: $(pwd)/wormhole-qt"
echo ""
echo "To install system-wide:"
echo "  sudo cmake --install ."
echo ""
echo "To run directly:"
echo "  ./wormhole-qt"
