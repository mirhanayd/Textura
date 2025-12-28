# PowerShell build script for MinGW/g++ available in PATH
Set-StrictMode -Version Latest

if (-Not (Test-Path build)) { New-Item -ItemType Directory -Path build | Out-Null }

Write-Host "Collecting source files..."
$png = Get-ChildItem -Path PNG -Filter *.c | ForEach-Object { $_.FullName }
$zlib = Get-ChildItem -Path zlib -Filter *.c | ForEach-Object { $_.FullName }

Write-Host "Compiling with g++ (PowerShell)..."
& g++ -std=c++17 -O2 -I. main.cpp $png $zlib -o build\textura_exe.exe

Write-Host "Built: build\textura_exe.exe"