# Making Comp76 Fully Independent
### No shared JUCE, no shared cache, no OneDrive throttle

---

## The Problem

Same issue as Gain76 and Filter76 — any restructure or new build directory triggers a full JUCE recompile from scratch, and OneDrive throttles I/O while object files are being written.

Fix: Comp76 owns its own JUCE copy, its own build directory, lives outside OneDrive.

---

## Step 1 — Clone JUCE Inside the Project

```bash
cd C:\Dev\Comp76
git clone https://github.com/juce-framework/JUCE.git
```

---

## Step 2 — CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.24)
project(Comp76 VERSION 1.0.0)

# Local JUCE — no external dependency
add_subdirectory(JUCE)

juce_add_plugin(Comp76
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Cmp1
    FORMATS VST3
    PRODUCT_NAME "Comp76"
)

target_sources(Comp76 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(Comp76 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Comp76 PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
```

> **Note:** `PLUGIN_CODE` must be unique across all your plugins. Running codes so far:
> - Gain76 → `Gain`
> - Filter76 → `Flt1`
> - Comp76 → `Cmp1`

---

## Step 3 — Build

```bash
cd C:\Dev\Comp76
cmake -B build -S .
cmake --build build --config Release
```

Output at:
```
Comp76\build\Comp76_artefacts\Release\VST3\Comp76.vst3
```

---

## Step 4 — Move Outside OneDrive

```
C:\Dev\
  ├── Gain76\
  ├── Filter76\
  └── Comp76\
```

---

## Final Folder Structure

```
C:\Dev\Comp76\
  ├── JUCE\
  ├── Source\
  │   ├── PluginProcessor.h
  │   ├── PluginProcessor.cpp
  │   ├── PluginEditor.h
  │   └── PluginEditor.cpp
  ├── CMakeLists.txt
  └── build\
      └── Comp76_artefacts\
          └── Release\
              └── VST3\
                  └── Comp76.vst3
```

---

## Commands Reference

```bash
# First-time setup
cd C:\Dev\Comp76
git clone https://github.com/juce-framework/JUCE.git
cmake -B build -S .
cmake --build build --config Release

# Subsequent builds
cmake --build build --config Release

# Clean rebuild
cmake --build build --config Release --clean-first

# Copy VST3 to DAW scan folder
xcopy /Y "build\Comp76_artefacts\Release\VST3\Comp76.vst3" "C:\Program Files\Common Files\VST3\" /E /I
```

---

## Steady-State Build Cycle

| Action | Rebuild cost |
|---|---|
| Edit a `.cpp` source file | That file only — fast (10–30 sec) |
| Edit a `.h` header | All files that include it |
| Bump version in `CMakeLists.txt` | Full rebuild — avoid unless needed |
| Add a new source file | CMake reconfigure + new file only |

---

## All Projects So Far

```
C:\Dev\
  ├── Gain76\        ← gain/volume test plugin       PLUGIN_CODE: Gain
  ├── Filter76\      ← biquad filter (LP, HP, EQ)    PLUGIN_CODE: Flt1
  └── Comp76\        ← compressor / dynamics          PLUGIN_CODE: Cmp1
```

Each project is fully self-contained. Touching one never affects the other.
