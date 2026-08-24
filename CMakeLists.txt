# ==============================================================================
# Pedalboard3 - CMake Build Configuration
#
# A modern VST3 plugin host preserving the original Pedalboard2 UI.
# Built on JUCE 8.0.15 with C++20.
# ==============================================================================

cmake_minimum_required(VERSION 3.24)

project(Pedalboard3
    VERSION 3.0.0
    DESCRIPTION "Modern VST3 Pedalboard Software for Live Performance"
    LANGUAGES C CXX
)

# ==============================================================================
# CPM.cmake - Dependency Manager
# ==============================================================================
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)

# ==============================================================================
# C++ Standard
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================================================================
# Modern C++ Libraries (via CPM)
# ==============================================================================

# fmt - Modern string formatting
CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 12.2.0
    OPTIONS "FMT_INSTALL OFF"
)

# spdlog - Fast async logging (uses fmt)
CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    GIT_TAG v1.17.0
    OPTIONS "SPDLOG_FMT_EXTERNAL ON" "SPDLOG_INSTALL OFF"
)

# nlohmann_json - JSON parsing
CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    GIT_TAG v3.12.0
    OPTIONS "JSON_BuildTests OFF" "JSON_Install OFF"
)

# ==============================================================================
# Testing Framework
# ==============================================================================
option(Pedalboard3_BUILD_TESTS "Build tests" ON)

if(Pedalboard3_BUILD_TESTS)
    CPMAddPackage(
        NAME Catch2
        GITHUB_REPOSITORY catchorg/Catch2
        GIT_TAG v3.15.3
    )
    if(Catch2_ADDED)
        list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    endif()
    enable_testing()
endif()

# ==============================================================================
# Compiler Warnings
# ==============================================================================
if(MSVC)
    add_compile_options(/W4 /WX-)  # High warnings, don't treat as errors yet
    add_compile_options(/wd4100)   # Unreferenced formal parameter (common in JUCE)
    add_compile_options(/wd4244)   # Conversion warnings (JUCE has many)
    add_compile_options(/wd4267)   # Size_t conversion (JUCE has many)
    add_compile_options(/FS)       # Force synchronous PDB writes
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options(-Wno-unused-parameter)
endif()

# ==============================================================================
# JUCE - Audio Framework (submodule at tag 8.0.15)
# ==============================================================================
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt)
    message(FATAL_ERROR
        "JUCE submodule not found. Run:\n"
        "  git submodule update --init --recursive\n"
        "Or clone JUCE manually:\n"
        "  git clone --depth 1 --branch 8.0.15 https://github.com/juce-framework/JUCE.git JUCE\n"
        "JUCE must be at tag 8.0.15 (NOT master/9.x - breaking API changes)."
    )
endif()

add_subdirectory(JUCE)

# ==============================================================================
# Main Application
# ==============================================================================
juce_add_gui_app(Pedalboard3
    PRODUCT_NAME "Pedalboard3"
    COMPANY_NAME "Pedalboard3 Project"
    BUNDLE_ID "com.pedalboard3.app"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/icon/icon-512.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/icon/icon-48.png"
)

# Generate JUCE header
juce_generate_juce_header(Pedalboard3)

# ==============================================================================
# Source Files
# ==============================================================================
# Phase 1: Core Audio/Graph Engine (Phase 3 UI files are in src/ and will be enabled once build errors are resolved)
target_sources(Pedalboard3 PRIVATE
    # Application
    src/App.cpp
    src/App.h

    # Audio Core
    src/AudioSingletons.cpp
    src/AudioSingletons.h
    src/FilterGraph.cpp
    src/FilterGraph.h
    src/InternalFilters.cpp
    src/InternalFilters.h
    src/BypassableInstance.cpp
    src/BypassableInstance.h
    src/MainTransport.cpp
    src/MainTransport.h
    src/MidiAppFifo.cpp
    src/MidiAppFifo.h

    # OSC (juce_osc replacement for NiallsOSCLib)
    src/Mapping.cpp
    src/Mapping.h
    src/OscMappingManager.cpp
    src/OscMappingManager.h
    src/TapTempoHelper.cpp
    src/TapTempoHelper.h

    # Phase 3 UI files have been ported to src/ but are temporarily disabled from
    # the build while their JUCE 8 API mismatches are being fixed. Re-enable by
    # adding them back to this list once they compile cleanly.

    # Group 1: Utilities
    src/PropertiesSingleton.cpp
    src/PropertiesSingleton.h
    src/LogFile.cpp
    src/LogFile.h
    src/JuceHelperStuff.cpp
    src/JuceHelperStuff.h
    src/Vectors.cpp
    src/Vectors.h
    src/Images.cpp
    src/Images.h
    src/LookAndFeelImages.cpp
    src/LookAndFeelImages.h

    # Group 2: LookAndFeel and colour scheme
    src/ColourScheme.cpp
    src/ColourScheme.h
    src/ColourSchemeEditor.cpp
    src/ColourSchemeEditor.h
    src/BranchesLAF.cpp
    src/BranchesLAF.h

    # Group 3: Built-in processors
    src/PedalboardProcessors.cpp
    src/PedalboardProcessors.h
    src/PedalboardProcessorEditors.cpp
    src/PedalboardProcessorEditors.h
    src/AudioRecorderControl.cpp
    src/AudioRecorderControl.h
    src/FilePlayerControl.cpp
    src/FilePlayerControl.h
    src/LooperControl.cpp
    src/LooperControl.h
    src/LooperEditor.cpp
    src/LooperEditor.h
    src/MetronomeControl.cpp
    src/MetronomeControl.h
    src/WaveformDisplay.cpp
    src/WaveformDisplay.h

    # Group 4: Plugin canvas
    src/PluginComponent.cpp
    src/PluginComponent.h
    src/PluginField.cpp
    src/PluginField.h

    # Group 5: Mappings
    src/MidiMappingManager.cpp
    src/MidiMappingManager.h
    src/MappingsDialog.cpp
    src/MappingsDialog.h
    src/MappingEntryMidi.cpp
    src/MappingEntryMidi.h
    src/MappingEntryOsc.cpp
    src/MappingEntryOsc.h
    src/MappingSlider.cpp
    src/MappingSlider.h
    src/MidiCcAlertWindow.cpp
    src/MidiCcAlertWindow.h
    src/ApplicationMappingsEditor.cpp
    src/ApplicationMappingsEditor.h

    # Group 6: Main UI
    src/MainPanel.cpp
    src/MainPanel.h
    src/AboutPage.cpp
    src/AboutPage.h
    src/LogDisplay.cpp
    src/LogDisplay.h
    src/TapTempoBox.cpp
    src/TapTempoBox.h
    src/PresetBar.cpp
    src/PresetBar.h

    # Group 7: Patch/preset management
    src/PatchOrganiser.cpp
    src/PatchOrganiser.h
    src/PresetManager.cpp
    src/PresetManager.h
    src/UserPresetWindow.cpp
    src/UserPresetWindow.h
    src/PreferencesDialog.cpp
    src/PreferencesDialog.h
    src/TrayIcon.cpp
    src/TrayIcon.h

    # Group 8: Stability infrastructure
    src/VuMeterDsp.h
    src/SafetyLimiter.cpp
    src/SafetyLimiter.h
    src/CrossfadeMixer.cpp
    src/CrossfadeMixer.h
    src/SettingsManager.cpp
    src/SettingsManager.h
    src/PluginBlacklist.cpp
    src/PluginBlacklist.h
    src/CrashProtection.cpp
    src/CrashProtection.h
    src/UndoActions.cpp
    src/UndoActions.h
    src/BlacklistWindow.cpp
    src/BlacklistWindow.h

    # Group 9: Scanner infrastructure
    src/PluginScannerIPC.h
    src/PluginScannerClient.cpp
    src/PluginScannerClient.h
    src/SafePluginScanner.cpp
    src/SafePluginScanner.h
    src/PluginPoolManager.cpp
    src/PluginPoolManager.h
)

# ==============================================================================
# Compile Definitions
# ==============================================================================
target_compile_definitions(Pedalboard3 PRIVATE
    # JUCE Settings
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Pedalboard3,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Pedalboard3,JUCE_VERSION>"

    # Plugin Hosting
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_VST=0
    JUCE_PLUGINHOST_AU=0
    JUCE_PLUGINHOST_LADSPA=1

    # Audio Settings (platform-specific below)
    JUCE_USE_FLAC=1
    JUCE_USE_OGGVORBIS=1

    # Modern features
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_STRICT_REFCOUNTEDPOINTER=1
)

# Platform-specific audio backends
if(WIN32)
    target_compile_definitions(Pedalboard3 PRIVATE
        JUCE_ASIO=1
        JUCE_WASAPI=1
        JUCE_DIRECTSOUND=1
        JUCE_WIN_PER_MONITOR_DPI_AWARE=1
    )
else()
    target_compile_definitions(Pedalboard3 PRIVATE
        JUCE_ALSA=1
        JUCE_JACK_AUDIO_DEVICES=1
    )
endif()

# ==============================================================================
# Link Libraries
# ==============================================================================
target_link_libraries(Pedalboard3 PRIVATE
    # JUCE modules
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_cryptography
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_opengl
    juce::juce_osc

    # Recommended flags
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags

    # Modern C++ libraries
    fmt::fmt
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

# Windows-specific settings
if(WIN32)
    target_compile_definitions(Pedalboard3 PRIVATE
        _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
    )
    target_link_libraries(Pedalboard3 PRIVATE ws2_32)
endif()

# ==============================================================================
# Out-of-Process Plugin Scanner
# ==============================================================================
juce_add_console_app(Pedalboard3Scanner
    PRODUCT_NAME "Pedalboard3Scanner"
    COMPANY_NAME "Pedalboard3 Project"
    BUNDLE_ID "com.pedalboard3.scanner"
)

juce_generate_juce_header(Pedalboard3Scanner)

target_sources(Pedalboard3Scanner PRIVATE
    src/PluginScannerMain.cpp
)

target_compile_definitions(Pedalboard3Scanner PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_LADSPA=1
    JUCE_MODAL_LOOPS_PERMITTED=1
)

if(WIN32)
    target_compile_definitions(Pedalboard3Scanner PRIVATE
        JUCE_ASIO=1
        JUCE_WASAPI=1
        JUCE_DIRECTSOUND=1
    )
else()
    target_compile_definitions(Pedalboard3Scanner PRIVATE
        JUCE_ALSA=1
        JUCE_JACK_AUDIO_DEVICES=1
    )
endif()

target_link_libraries(Pedalboard3Scanner PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    fmt::fmt
    spdlog::spdlog
)

# ==============================================================================
# Tests
# ==============================================================================
if(Pedalboard3_BUILD_TESTS)
    add_subdirectory(tests)
endif()
