# WaveBombz Endless Smile

A JUCE-based VST3 audio effect built around a single Intensity macro and four character presets.

## Current status

This repository contains the initial project scaffold. Build and plugin validation are handled by the GitHub Actions workflow in `.github/workflows/build.yml`.

## Requirements

- CMake 3.22 or newer
- A C++17 compiler
- JUCE 8 (fetched by CMake)

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target WaveBombzEndlessSmile_VST3
```

The project uses JUCE's CMake integration and FetchContent to retrieve JUCE. The first configure requires network access.

## Parameters

- **Intensity**: 0–100%, default 0%
- **Character**: Classic, Massive, Tech, Atmosphere

The effect combines a resonant high-pass filter, tempo-synchronised dotted-eighth stereo delay, reverb, and soft saturation. Intensity is smoothed to avoid abrupt parameter changes.

## License

All rights reserved unless otherwise specified by the repository owner.
