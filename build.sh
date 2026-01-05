#!/bin/bash
# Cleveland Sound Hothouse Pedal - Build Script
# 
# This script compiles the effect pedal code for deployment

set -e

echo "Building Cleveland Sound Hothouse Pedal Effects..."

# Configuration
OUTPUT_DIR="build"
COMPILER="g++"
CFLAGS="-std=c++11 -O2 -Wall -I."

# Create build directory
mkdir -p $OUTPUT_DIR

# Compile the deployment example
echo "Compiling deployment example..."
$COMPILER $CFLAGS deploy.cpp -o $OUTPUT_DIR/hothouse_pedal -lm

echo "Build complete!"
echo "Output: $OUTPUT_DIR/hothouse_pedal"
echo ""
echo "To run the example: ./$OUTPUT_DIR/hothouse_pedal"
