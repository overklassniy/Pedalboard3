# Pedalboard3 - CMake Build Configuration
#
# A modern VST3 plugin host preserving the original Pedalboard2 UI.
# Built on JUCE 8.0.15 with C++20.

cmake_minimum_required(VERSION 3.24)

# The project version is used by JUCE as the application version, which the
# About dialog's GitHub update check compares against the latest release tag.
# CI release builds pass -DPedalboard3_VERSION_OVERRIDE=<version> so that a
# tagged build reports the tag version instead of the hardcoded default.
#
# CMake's project(VERSION ...) requires a numeric MAJOR.MINOR.PATCH[.TWEAK]
# format. Version strings with a pre-release suffix (e.g., "0.0.0-test",
# "3.0.0-beta1") are rejected. To support such suffixes, the numeric prefix
# is extracted for project(VERSION ...), and the full string is stored in
# _pedalboard3_display_version for later use as the JUCE_VERSION target
# property (see the set_target_properties call after juce_add_gui_app).
if(DEFINED Pedalboard3_VERSION_OVERRIDE)
    string(REGEX MATCH "^([0-9]+(\\.[0-9]+)*)"
           _pedalboard3_cmake_version "${Pedalboard3_VERSION_OVERRIDE}")
    if(NOT _pedalboard3_cmake_version)
        set(_pedalboard3_cmake_version "0.0.0")
    endif()
    set(_pedalboard3_display_version "${Pedalboard3_VERSION_OVERRIDE}")
else()
    set(_pedalboard3_cmake_version "3.0.0")
    set(_pedalboard3_display_version "3.0.0")
endif()

project(Pedalboard3
    VERSION "${_pedalboard3_cmake_version}"
    DESCRIPTION "Modern VST3 Pedalboard Software for Live Performance"
    LANGUAGES C CXX
)

# CPM.cmake - Dependency Manager
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Modern C++ Libraries (via CPM)

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

# Testing Framework
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

# Compiler Warnings
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

# JUCE - Audio Framework (submodule at tag 8.0.15)
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

# Main Application
juce_add_gui_app(Pedalboard3
    PRODUCT_NAME "Pedalboard3"
    COMPANY_NAME "Pedalboard3 Project"
    BUNDLE_ID "com.pedalboard3.app"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/icon-512.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/icon-48.png"
)

# Override the JUCE version with the full display version, which may include
# a pre-release suffix (e.g., "0.0.0-test") that CMake's project(VERSION)
# rejects. JUCE sets JUCE_VERSION from PROJECT_VERSION via
# _juce_set_property_if_not_set, so this override takes effect. The
# JUCE_APPLICATION_VERSION_STRING compile definition reads this property at
# build time via a generator expression, so the About dialog shows the
# complete version string.
set_target_properties(Pedalboard3 PROPERTIES
    JUCE_VERSION "${_pedalboard3_display_version}")

# Generate JUCE header
juce_generate_juce_header(Pedalboard3)

# Embedded user documentation (F1 help).
# Single self-contained HTML file; written to the app data folder on F1
# and launched in the default browser. See assets/help/README.md.
juce_add_binary_data(Pedalboard3Help
    NAMESPACE HelpData
    HEADER_NAME HelpData.h
    SOURCES assets/help/help.html
)

# Source Files
target_sources(Pedalboard3 PRIVATE
    # Application shell
    src/app/App.cpp
    src/app/App.h
    src/app/MainPanel.cpp
    src/app/MainPanel.h
    src/app/AboutPage.cpp
    src/app/AboutPage.h
    src/app/LogDisplay.cpp
    src/app/LogDisplay.h
    src/app/TrayIcon.cpp
    src/app/TrayIcon.h

    # Audio core
    src/audio/AudioSingletons.cpp
    src/audio/AudioSingletons.h
    src/audio/FilterGraph.cpp
    src/audio/FilterGraph.h
    src/audio/InternalFilters.cpp
    src/audio/InternalFilters.h
    src/audio/BypassableInstance.cpp
    src/audio/BypassableInstance.h
    src/audio/MainTransport.cpp
    src/audio/MainTransport.h
    src/audio/MidiAppFifo.cpp
    src/audio/MidiAppFifo.h

    # OSC mapping
    src/osc/Mapping.cpp
    src/osc/Mapping.h
    src/osc/OscMappingManager.cpp
    src/osc/OscMappingManager.h
    src/osc/TapTempoHelper.cpp
    src/osc/TapTempoHelper.h

    # Utilities and embedded resources
    src/util/PropertiesSingleton.cpp
    src/util/PropertiesSingleton.h
    src/util/LogFile.cpp
    src/util/LogFile.h
    src/util/JuceHelperStuff.cpp
    src/util/JuceHelperStuff.h
    src/util/Vectors.cpp
    src/util/Vectors.h
    src/util/Images.cpp
    src/util/Images.h
    src/util/LookAndFeelImages.cpp
    src/util/LookAndFeelImages.h

    # LookAndFeel and colour scheme
    src/lookandfeel/BranchesLAF.cpp
    src/lookandfeel/BranchesLAF.h
    src/lookandfeel/ColourScheme.cpp
    src/lookandfeel/ColourScheme.h
    src/lookandfeel/ColourSchemeEditor.cpp
    src/lookandfeel/ColourSchemeEditor.h

    # Built-in processors (split into per-class files)
    src/processors/PedalboardProcessor.h
    src/processors/LevelProcessor.cpp
    src/processors/LevelProcessor.h
    src/processors/FilePlayerProcessor.cpp
    src/processors/FilePlayerProcessor.h
    src/processors/OutputToggleProcessor.cpp
    src/processors/OutputToggleProcessor.h
    src/processors/VuMeterProcessor.cpp
    src/processors/VuMeterProcessor.h
    src/processors/RecorderProcessor.cpp
    src/processors/RecorderProcessor.h
    src/processors/MetronomeProcessor.cpp
    src/processors/MetronomeProcessor.h
    src/processors/LooperProcessor.cpp
    src/processors/LooperProcessor.h
    src/processors/LevelEditor.cpp
    src/processors/LevelEditor.h
    src/processors/FilePlayerEditor.cpp
    src/processors/FilePlayerEditor.h
    src/processors/OutputToggleEditor.cpp
    src/processors/OutputToggleEditor.h
    src/processors/VuMeterEditor.cpp
    src/processors/VuMeterEditor.h
    src/processors/AudioRecorderEditor.cpp
    src/processors/AudioRecorderEditor.h
    src/processors/MetronomeEditor.cpp
    src/processors/MetronomeEditor.h
    src/processors/PedalboardProcessors.cpp
    src/processors/PedalboardProcessors.h
    src/processors/PedalboardProcessorEditors.cpp
    src/processors/PedalboardProcessorEditors.h
    src/processors/AudioRecorderControl.cpp
    src/processors/AudioRecorderControl.h
    src/processors/FilePlayerControl.cpp
    src/processors/FilePlayerControl.h
    src/processors/LooperControl.cpp
    src/processors/LooperControl.h
    src/processors/LooperEditor.cpp
    src/processors/LooperEditor.h
    src/processors/MetronomeControl.cpp
    src/processors/MetronomeControl.h
    src/processors/WaveformDisplay.cpp
    src/processors/WaveformDisplay.h
    src/processors/VuMeterDsp.h
    src/processors/SafetyLimiter.cpp
    src/processors/SafetyLimiter.h
    src/processors/CrossfadeMixer.cpp
    src/processors/CrossfadeMixer.h

    # Plugin canvas
    src/canvas/PluginComponent.cpp
    src/canvas/PluginComponent.h
    src/canvas/PluginField.cpp
    src/canvas/PluginField.h

    # Mappings
    src/mappings/MidiMappingManager.cpp
    src/mappings/MidiMappingManager.h
    src/mappings/MappingsDialog.cpp
    src/mappings/MappingsDialog.h
    src/mappings/MappingEntryMidi.cpp
    src/mappings/MappingEntryMidi.h
    src/mappings/MappingEntryOsc.cpp
    src/mappings/MappingEntryOsc.h
    src/mappings/MappingSlider.cpp
    src/mappings/MappingSlider.h
    src/mappings/MidiCcAlertWindow.cpp
    src/mappings/MidiCcAlertWindow.h
    src/mappings/ApplicationMappingsEditor.cpp
    src/mappings/ApplicationMappingsEditor.h
    src/mappings/TapTempoBox.cpp
    src/mappings/TapTempoBox.h

    # Patch and preset management
    src/preset/PatchOrganiser.cpp
    src/preset/PatchOrganiser.h
    src/preset/PresetManager.cpp
    src/preset/PresetManager.h
    src/preset/PresetBar.cpp
    src/preset/PresetBar.h
    src/preset/UserPresetWindow.cpp
    src/preset/UserPresetWindow.h
    src/preset/PreferencesDialog.cpp
    src/preset/PreferencesDialog.h

    # Stability infrastructure
    src/stability/SettingsManager.cpp
    src/stability/SettingsManager.h
    src/stability/PluginBlacklist.cpp
    src/stability/PluginBlacklist.h
    src/stability/CrashProtection.cpp
    src/stability/CrashProtection.h
    src/stability/UndoActions.cpp
    src/stability/UndoActions.h
    src/stability/BlacklistWindow.cpp
    src/stability/BlacklistWindow.h

    # Scanner infrastructure
    src/scanner/PluginScannerIPC.h
    src/scanner/PluginScannerClient.cpp
    src/scanner/PluginScannerClient.h
    src/scanner/SafePluginScanner.cpp
    src/scanner/SafePluginScanner.h
    src/scanner/PluginPoolManager.cpp
    src/scanner/PluginPoolManager.h
)

# Include Directories (flat includes work across all subdirectories)
target_include_directories(Pedalboard3 PRIVATE
    src/app
    src/audio
    src/osc
    src/util
    src/lookandfeel
    src/processors
    src/canvas
    src/mappings
    src/preset
    src/stability
    src/scanner
)

# Compile Definitions
target_compile_definitions(Pedalboard3 PRIVATE
    # JUCE Settings
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Pedalboard3,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Pedalboard3,JUCE_VERSION>"

    # Plugin Hosting
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_VST=1
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

# Link Libraries
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

    # Embedded user documentation (F1 help)
    Pedalboard3Help
)

# Windows-specific settings
if(WIN32)
    target_compile_definitions(Pedalboard3 PRIVATE
        _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
    )
    target_link_libraries(Pedalboard3 PRIVATE ws2_32)
endif()

# Out-of-Process Plugin Scanner
juce_add_console_app(Pedalboard3Scanner
    PRODUCT_NAME "Pedalboard3Scanner"
    COMPANY_NAME "Pedalboard3 Project"
    BUNDLE_ID "com.pedalboard3.scanner"
)

juce_generate_juce_header(Pedalboard3Scanner)

target_sources(Pedalboard3Scanner PRIVATE
    src/scanner/PluginScannerMain.cpp
)

target_include_directories(Pedalboard3Scanner PRIVATE
    src/scanner
)

target_compile_definitions(Pedalboard3Scanner PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_VST=1
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

# Copy the scanner binary next to the main executable so that
# PluginScannerClient can find it at runtime.
add_custom_command(TARGET Pedalboard3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Pedalboard3Scanner>
        $<TARGET_FILE_DIR:Pedalboard3>/Pedalboard3Scanner.exe
)

# Tests
if(Pedalboard3_BUILD_TESTS)
    add_subdirectory(tests)
endif()

unset(_pedalboard3_cmake_version)
unset(_pedalboard3_display_version)
