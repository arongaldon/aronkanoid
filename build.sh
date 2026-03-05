#!/bin/bash

# Exit on any error
set -e

echo "Installing required dependencies for Raylib..."
sudo apt-get update
sudo apt-get install -y libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev

echo "Configuring CMake..."
mkdir -p build
cd build
cmake ..

echo "Building Aronkanoid..."
make -j$(nproc)

echo "Build complete. You can run the game with: ./build/Aronkanoid"
