// PluginBlacklist.h - User-configurable plugin blacklist for crash protection.
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
#include <mutex>
#include <set>

/// Manages a user-configurable list of blacklisted plugins.
///
/// Plugins on the blacklist are skipped during scanning and not loaded.
/// The blacklist persists across sessions via SettingsManager.
class PluginBlacklist {
  public:
    /// Get the singleton instance
    ///
    /// @return The global PluginBlacklist instance.
    static PluginBlacklist& getInstance();

    /// Check if a plugin path is blacklisted
    ///
    /// @param pluginPath Path of the plugin to check.
    /// @return True if the plugin path is blacklisted.
    bool isBlacklisted(const juce::String& pluginPath) const;

    /// Check if a plugin identifier is blacklisted
    ///
    /// @param pluginId Identifier of the plugin to check.
    /// @return True if the plugin identifier is blacklisted.
    bool isBlacklistedById(const juce::String& pluginId) const;

    /// Add a plugin to the blacklist by path
    ///
    /// @param pluginPath Path of the plugin to blacklist.
    void addToBlacklist(const juce::String& pluginPath);

    /// Add a plugin to the blacklist by identifier
    ///
    /// @param pluginId Identifier of the plugin to blacklist.
    void addToBlacklistById(const juce::String& pluginId);

    /// Remove a plugin from the blacklist by path
    ///
    /// @param pluginPath Path of the plugin to remove.
    void removeFromBlacklist(const juce::String& pluginPath);

    /// Remove a plugin from the blacklist by identifier
    ///
    /// @param pluginId Identifier of the plugin to remove.
    void removeFromBlacklistById(const juce::String& pluginId);

    /// Get all blacklisted paths
    ///
    /// @return Array of all blacklisted plugin paths.
    juce::StringArray getBlacklistedPaths() const;

    /// Get all blacklisted plugin identifiers
    ///
    /// @return Array of all blacklisted plugin identifiers.
    juce::StringArray getBlacklistedIds() const;

    /// Clear the entire blacklist
    void clearBlacklist();

    /// Get count of blacklisted plugins
    ///
    /// @return Total number of blacklisted plugins (paths plus identifiers).
    int getBlacklistCount() const;

    /// Load blacklist from settings
    void loadFromSettings();

    /// Save blacklist to settings
    void saveToSettings();

  private:
    PluginBlacklist();
    ~PluginBlacklist() = default;

    PluginBlacklist(const PluginBlacklist&) = delete;
    PluginBlacklist& operator=(const PluginBlacklist&) = delete;

    /// Normalizes a path for case-insensitive comparison on Windows.
    ///
    /// @param path Plugin path to normalize.
    /// @return Normalized path string.
    juce::String normalizePath(const juce::String& path) const;

    /// Blacklisted plugin paths (normalized).
    std::set<juce::String> blacklistedPaths;
    /// Blacklisted plugin identifiers.
    std::set<juce::String> blacklistedIds;
    /// Guards access to the blacklist sets.
    mutable std::mutex blacklistMutex;
};
