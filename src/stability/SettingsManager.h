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
/// Settings are stored as JSON and auto-saved to disk after each write.
///
/// Usage:
/// @code
/// // Get a setting
/// bool audioEnabled = SettingsManager::getInstance().getBool("AudioInput", true);
///
/// // Set a setting (auto-saves to disk)
/// SettingsManager::getInstance().setValue("AudioInput", false);
/// @endcode
class SettingsManager {
  public:
    /// Returns the singleton instance.
    ///
    /// @return The global SettingsManager instance.
    static SettingsManager& getInstance();

    /// Loads settings from the JSON file.
    ///
    /// Called automatically on first access. Safe to call multiple times.
    void load();

    /// Saves settings to the JSON file.
    ///
    /// Called automatically after each setValue() call.
    void save();

    // Typed getters

    /// Returns the string for key, or defaultValue when absent.
    ///
    /// @param key Settings key to look up.
    /// @param defaultValue Value to return when the key is absent.
    /// @return The string value for key, or defaultValue when absent.
    juce::String getString(const std::string& key, const juce::String& defaultValue = {}) const;

    /// Returns the boolean for key, or defaultValue when absent.
    ///
    /// @param key Settings key to look up.
    /// @param defaultValue Value to return when the key is absent.
    /// @return The boolean value for key, or defaultValue when absent.
    bool getBool(const std::string& key, bool defaultValue = false) const;

    /// Returns the integer for key, or defaultValue when absent.
    ///
    /// @param key Settings key to look up.
    /// @param defaultValue Value to return when the key is absent.
    /// @return The integer value for key, or defaultValue when absent.
    int getInt(const std::string& key, int defaultValue = 0) const;

    /// Returns the double for key, or defaultValue when absent.
    ///
    /// @param key Settings key to look up.
    /// @param defaultValue Value to return when the key is absent.
    /// @return The double value for key, or defaultValue when absent.
    double getDouble(const std::string& key, double defaultValue = 0.0) const;

    /// Returns the string array stored as a JSON array for key, or empty if absent.
    ///
    /// @param key Settings key to look up.
    /// @return The string array for key, or empty if absent.
    juce::StringArray getStringArray(const std::string& key) const;

    // Setters (auto-save after each call)

    /// Sets a string value for key.
    ///
    /// @param key Settings key to set.
    /// @param value String value to store.
    void setValue(const std::string& key, const juce::String& value);

    /// Sets a boolean value for key.
    ///
    /// @param key Settings key to set.
    /// @param value Boolean value to store.
    void setValue(const std::string& key, bool value);

    /// Sets an integer value for key.
    ///
    /// @param key Settings key to set.
    /// @param value Integer value to store.
    void setValue(const std::string& key, int value);

    /// Sets a double value for key.
    ///
    /// @param key Settings key to set.
    /// @param value Double value to store.
    void setValue(const std::string& key, double value);

    /// Sets an XmlElement for key, stored as its string representation.
    ///
    /// @param key Settings key to set.
    /// @param xml XmlElement to serialize and store.
    void setValue(const std::string& key, const juce::XmlElement& xml);

    /// Sets a string array for key, stored as a JSON array.
    ///
    /// @param key Settings key to set.
    /// @param value String array to store.
    void setStringArray(const std::string& key, const juce::StringArray& value);

    /// Initializes the settings system.
    ///
    /// Creates the settings directory if needed and loads existing settings.
    void initialise();

    /// Returns the user data directory (%APPDATA%/Pedalboard3 or equivalent).
    ///
    /// @return The user data directory for Pedalboard3.
    juce::File getUserDataDirectory() const;

    /// Returns the XmlElement stored for key, or nullptr if not found.
    ///
    /// @param key Settings key to look up.
    /// @return The XmlElement for key, or nullptr if not found.
    std::unique_ptr<juce::XmlElement> getXmlValue(const std::string& key) const;

  private:
    SettingsManager();
    ~SettingsManager() = default;

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    /// Returns the path to the JSON settings file.
    ///
    /// @return Filesystem path to the settings JSON file.
    std::filesystem::path getSettingsFilePath() const;

    /// In-memory cache of the parsed settings JSON.
    nlohmann::json settingsData;
    /// Guards access to the settings cache.
    mutable std::mutex settingsMutex;
    /// True when there are unsaved changes pending a write.
    bool needsSaving = false;
};
