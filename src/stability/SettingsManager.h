// SettingsManager.h - Modern JSON-based settings persistence.
//
// This file is part of Pedalboard3, an audio plugin host.
// Ported from the Pedalboard3-VST3 fork by Project12x.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <JuceHeader.h>
#include <filesystem>
#include <mutex>
#include <nlohmann/json.hpp>

/// Thread-safe singleton for application settings.
///
/// Usage:
/// @code
/// // Get a setting
/// bool audioEnabled = SettingsManager::getInstance().getBool("AudioInput", true);
///
/// // Set a setting (auto-saves to disk)
/// SettingsManager::getInstance().setValue("AudioInput", false);
/// @endcode
class SettingsManager
{
  public:
    /// Get the singleton instance.
    /// @return Reference to the SettingsManager singleton.
    static SettingsManager& getInstance();

    /// Load settings from JSON file.
    /// Called automatically on first access. Safe to call multiple times.
    void load();

    /// Save settings to JSON file.
    /// Called automatically after each setValue() call.
    void save();

    // Typed Getters

    /// Get a string value.
    juce::String getString(const std::string& key, const juce::String& defaultValue = {}) const;

    /// Get a boolean value.
    bool getBool(const std::string& key, bool defaultValue = false) const;

    /// Get an integer value.
    int getInt(const std::string& key, int defaultValue = 0) const;

    /// Get a double value.
    double getDouble(const std::string& key, double defaultValue = 0.0) const;

    /// Get a string array (stored as JSON array).
    juce::StringArray getStringArray(const std::string& key) const;

    // Setters (auto-save after each call)

    /// Set a string value.
    void setValue(const std::string& key, const juce::String& value);

    /// Set a boolean value.
    void setValue(const std::string& key, bool value);

    /// Set an integer value.
    void setValue(const std::string& key, int value);

    /// Set a double value.
    void setValue(const std::string& key, double value);

    /// Set an XML element (stored as string).
    void setValue(const std::string& key, const juce::XmlElement& xml);

    /// Set a string array (stored as JSON array).
    void setStringArray(const std::string& key, const juce::StringArray& value);

    /// Initialize the settings system.
    /// Creates the settings directory if needed and loads existing settings.
    void initialise();

    /// Get the user data directory.
    /// @return Path to %APPDATA%/Pedalboard3 (or equivalent).
    juce::File getUserDataDirectory() const;

    /// Get an XML value from settings.
    /// @param key The setting key.
    /// @return Parsed XmlElement, or nullptr if not found.
    std::unique_ptr<juce::XmlElement> getXmlValue(const std::string& key) const;

  private:
    SettingsManager();
    ~SettingsManager() = default;

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    std::filesystem::path getSettingsFilePath() const;

    nlohmann::json settingsData;      ///< In-memory settings cache
    mutable std::mutex settingsMutex; ///< Thread safety for settings access
    bool needsSaving = false;         ///< Dirty flag for pending saves
};

