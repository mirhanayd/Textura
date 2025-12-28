#!/usr/bin/env bash
set -e
# Build script for WSL / MSYS2 / bash environments
mkdir -p build

echo "Compiling with g++ (bash/WSL)..."

g++ -std=c++17 -O2 -I. main.cpp PNG/*.c zlib/*.c -o build/textura_exe

echo "Built: build/textura_exe" 
