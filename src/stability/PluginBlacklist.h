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
class PluginBlacklist
{
  public:
    /// Get the singleton instance
    static PluginBlacklist& getInstance();

    /// Check if a plugin path is blacklisted
    bool isBlacklisted(const juce::String& pluginPath) const;

    /// Check if a plugin identifier is blacklisted
    bool isBlacklistedById(const juce::String& pluginId) const;

    /// Add a plugin to the blacklist by path
    void addToBlacklist(const juce::String& pluginPath);

    /// Add a plugin to the blacklist by identifier
    void addToBlacklistById(const juce::String& pluginId);

    /// Remove a plugin from the blacklist by path
    void removeFromBlacklist(const juce::String& pluginPath);

    /// Remove a plugin from the blacklist by identifier
    void removeFromBlacklistById(const juce::String& pluginId);

    /// Get all blacklisted paths
    juce::StringArray getBlacklistedPaths() const;

    /// Get all blacklisted plugin identifiers
    juce::StringArray getBlacklistedIds() const;

    /// Clear the entire blacklist
    void clearBlacklist();

    /// Get count of blacklisted plugins
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

    /// Normalize path for case-insensitive comparison on Windows
    juce::String normalizePath(const juce::String& path) const;

    std::set<juce::String> blacklistedPaths; ///< Paths (normalized)
    std::set<juce::String> blacklistedIds;   ///< Plugin identifiers
    mutable std::mutex blacklistMutex;       ///< Thread safety
};
