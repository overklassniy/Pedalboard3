// PluginBlacklist.cpp - User-configurable plugin blacklist for crash protection.
//
// This file is part of Pedalboard3, an audio plugin host.
// Ported and modified from the Pedalboard3 fork by Project12x.
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

#include "PluginBlacklist.h"

#include "SettingsManager.h"

#include <spdlog/spdlog.h>

PluginBlacklist& PluginBlacklist::getInstance() {
    static PluginBlacklist instance;
    return instance;
}

PluginBlacklist::PluginBlacklist() {
    loadFromSettings();
}

juce::String PluginBlacklist::normalizePath(const juce::String& path) const {
#if JUCE_WINDOWS
    // Case-insensitive on Windows, also normalize slashes
    return path.toLowerCase().replace("\\", "/");
#else
    return path;
#endif
}

bool PluginBlacklist::isBlacklisted(const juce::String& pluginPath) const {
    std::lock_guard<std::mutex> lock(blacklistMutex);
    return blacklistedPaths.count(normalizePath(pluginPath)) > 0;
}

bool PluginBlacklist::isBlacklistedById(const juce::String& pluginId) const {
    std::lock_guard<std::mutex> lock(blacklistMutex);
    return blacklistedIds.count(pluginId) > 0;
}

void PluginBlacklist::addToBlacklist(const juce::String& pluginPath) {
    {
        std::lock_guard<std::mutex> lock(blacklistMutex);
        blacklistedPaths.insert(normalizePath(pluginPath));
    }
    spdlog::info("[PluginBlacklist] Added path to blacklist: {}", pluginPath.toStdString());
    saveToSettings();
}

void PluginBlacklist::addToBlacklistById(const juce::String& pluginId) {
    {
        std::lock_guard<std::mutex> lock(blacklistMutex);
        blacklistedIds.insert(pluginId);
    }
    spdlog::info("[PluginBlacklist] Added ID to blacklist: {}", pluginId.toStdString());
    saveToSettings();
}

void PluginBlacklist::removeFromBlacklist(const juce::String& pluginPath) {
    {
        std::lock_guard<std::mutex> lock(blacklistMutex);
        blacklistedPaths.erase(normalizePath(pluginPath));
    }
    spdlog::info("[PluginBlacklist] Removed path from blacklist: {}", pluginPath.toStdString());
    saveToSettings();
}

void PluginBlacklist::removeFromBlacklistById(const juce::String& pluginId) {
    {
        std::lock_guard<std::mutex> lock(blacklistMutex);
        blacklistedIds.erase(pluginId);
    }
    spdlog::info("[PluginBlacklist] Removed ID from blacklist: {}", pluginId.toStdString());
    saveToSettings();
}

juce::StringArray PluginBlacklist::getBlacklistedPaths() const {
    std::lock_guard<std::mutex> lock(blacklistMutex);
    juce::StringArray result;
    for (const auto& path : blacklistedPaths)
        result.add(path);
    return result;
}

juce::StringArray PluginBlacklist::getBlacklistedIds() const {
    std::lock_guard<std::mutex> lock(blacklistMutex);
    juce::StringArray result;
    for (const auto& id : blacklistedIds)
        result.add(id);
    return result;
}

void PluginBlacklist::clearBlacklist() {
    {
        std::lock_guard<std::mutex> lock(blacklistMutex);
        blacklistedPaths.clear();
        blacklistedIds.clear();
    }
    spdlog::info("[PluginBlacklist] Blacklist cleared");
    saveToSettings();
}

int PluginBlacklist::getBlacklistCount() const {
    std::lock_guard<std::mutex> lock(blacklistMutex);
    return static_cast<int>(blacklistedPaths.size() + blacklistedIds.size());
}

void PluginBlacklist::loadFromSettings() {
    std::lock_guard<std::mutex> lock(blacklistMutex);

    blacklistedPaths.clear();
    blacklistedIds.clear();

    auto& settings = SettingsManager::getInstance();

    // Load paths
    juce::StringArray paths = settings.getStringArray("pluginBlacklistPaths");
    for (const auto& path : paths)
        blacklistedPaths.insert(normalizePath(path));

    // Load IDs
    juce::StringArray ids = settings.getStringArray("pluginBlacklistIds");
    for (const auto& id : ids)
        blacklistedIds.insert(id);

    spdlog::debug("[PluginBlacklist] Loaded {} paths, {} IDs from settings", blacklistedPaths.size(),
                  blacklistedIds.size());
}

void PluginBlacklist::saveToSettings() {
    juce::StringArray paths;
    juce::StringArray ids;

    {
        std::lock_guard<std::mutex> lock(blacklistMutex);
        for (const auto& path : blacklistedPaths)
            paths.add(path);
        for (const auto& id : blacklistedIds)
            ids.add(id);
    }

    auto& settings = SettingsManager::getInstance();
    settings.setStringArray("pluginBlacklistPaths", paths);
    settings.setStringArray("pluginBlacklistIds", ids);

    spdlog::debug("[PluginBlacklist] Saved {} paths, {} IDs to settings", paths.size(), ids.size());
}
