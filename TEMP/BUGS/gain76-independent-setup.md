# Making Gain76 Fully Independent
### No shared JUCE, no shared cache, no OneDrive throttle

---

## The Problem

When CMake restructures into a new project directory (`Gain76/`) with a new build directory (`build-gain76/`), you pay a full recompile cost every time because:

1. **Fresh CMake configure** — JUCE reconfigures from scratch (~80 seconds)
2. **Full JUCE recompile** — zero cached object files, every framework module rebuilds from nothing
3. **OneDrive** — scans all object file churn as it's written, throttling disk I/O

The fix is to make `Gain76/` own everything — its own JUCE copy, its own build directory, outside OneDrive.

---

## Step 1 — Clone JUCE Inside the Project

Instead of pointing to a shared JUCE elsewhere on your system, put JUCE directly inside `Gain76/`:

```bash
cd Gain76
git clone https://github.com/juce-framework/JUCE.git
```

This means `Gain76/` carries its own JUCE — no dependency on anything outside the folder.

---

## Step 2 — Update CMakeLists.txt

Change the JUCE path to point to the local copy:

```cmake
cmake_minimum_required(VERSION 3.24)
project(Gain76 VERSION 1.0.0)

# Local JUCE — no external dependency
add_subdirectory(JUCE)

juce_add_plugin(Gain76
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Gain
    FORMATS VST3
    PRODUCT_NAME "Gain76"
)

target_sources(Gain76 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(Gain76 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Gain76 PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
```

---

## Step 3 — Move Build Directory Inside the Project

Keep the build folder self-contained inside `Gain76/`:

```bash
cd Gain76
cmake -B build -S .
cmake --build build --config Release
```

The `.vst3` output will be at:
```
Gain76/build/Gain76_artefacts/Release/VST3/Gain76.vst3
```

---

## Step 4 — Kill OneDrive Throttling

OneDrive scanning object files during compilation kills build speed. Two options:

### Option A — Move the whole project outside OneDrive (recommended)
Move `Gain76/` to somewhere like:
```
C:\Dev\Gain76\
```
Anything outside `C:\Users\YourName\OneDrive\` is not synced.

### Option B — Exclude just the build folder
If you want to keep the project in OneDrive but exclude the build output:

1. Open **OneDrive Settings** → **Backup** → **Manage backup**
2. Or right-click `Gain76\build\` in Explorer
3. Add to OneDrive exclusions

Alternatively, add a `.onedriveignore` or move the build dir to a non-synced location with:

```cmake
set(CMAKE_BINARY_DIR "C:/Dev/builds/Gain76")
```

---

## Final Folder Structure

```
C:\Dev\Gain76\               ← outside OneDrive
  ├── JUCE\                  ← local JUCE clone
  ├── Source\
  │   ├── PluginProcessor.h
  │   ├── PluginProcessor.cpp
  │   ├── PluginEditor.h
  │   └── PluginEditor.cpp
  ├── CMakeLists.txt
  └── build\                 ← build output, not synced
      └── Gain76_artefacts\
          └── Release\
              └── VST3\
                  └── Gain76.vst3
```

---

## Steady-State Build Cycle After This

Once set up, the only recompile triggers are:

| Action | Rebuild cost |
|---|---|
| Edit a `.cpp` source file | Recompiles that file only — fast |
| Edit a `.h` header | Recompiles all files that include it |
| Bump version in `CMakeLists.txt` | Touches every file — heavy rebuild |
| Change `cmake_minimum_required` | Full reconfigure |
| Add a new source file | CMake reconfigure + compile new file |

**Avoid bumping version unnecessarily** — it triggers a full rebuild every time.

---

## Commands Reference

```bash
# First-time setup
cd C:\Dev\Gain76
git clone https://github.com/juce-framework/JUCE.git
cmake -B build -S .
cmake --build build --config Release

# Subsequent builds (after editing source files)
cmake --build build --config Release

# Clean rebuild from scratch
cmake --build build --config Release --clean-first

# Copy VST3 to DAW scan folder
xcopy /Y "build\Gain76_artefacts\Release\VST3\Gain76.vst3" "C:\Program Files\Common Files\VST3\" /E /I
```
