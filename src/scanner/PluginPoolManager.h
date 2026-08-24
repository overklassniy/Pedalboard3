// PluginPoolManager.h - Sliding window plugin pool for instant patch switching.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2024-2026 Antigravity.
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

#ifndef PLUGINPOOLMANAGER_H_
#define PLUGINPOOLMANAGER_H_

#include <JuceHeader.h>
#include <map>
#include <memory>
#include <set>
#include <vector>

class FilterGraph;

/// A cached plugin instance with its state.
struct PooledPlugin {
    std::unique_ptr<AudioPluginInstance> instance;
    PluginDescription description;
    /// True when the plugin is currently in use by the active patch.
    bool isActive = false;
    /// Number of patches in the preload window that need this plugin.
    int refCount = 0;
    Time lastUsed;
};

/// Listener interface for pool loading progress notifications.
class PluginPoolListener {
  public:
    virtual ~PluginPoolListener() = default;

    /// Called when a patch's plugins are being loaded.
    ///
    /// @param patchIndex Index of the patch being loaded.
    /// @param progress Loading progress from 0.0 to 1.0.
    virtual void patchLoadingProgress(int patchIndex, float progress) = 0;

    /// Called when a patch is fully loaded and ready.
    ///
    /// @param patchIndex Index of the patch that is now ready.
    virtual void patchReady(int patchIndex) = 0;
};

/// Manages a sliding window pool of preloaded plugins for instant patch switching.
///
/// Instead of loading/unloading entire patches, this maintains a live pool of
/// plugins for the current patch plus N patches ahead/behind in the setlist.
/// This matches the Gig Performer architecture for zero-gap switching.
class PluginPoolManager : private Thread {
  public:
    // Singleton access

    /// Gets the global PluginPoolManager instance.
    static PluginPoolManager& getInstance();

    /// Shuts down and destroys the singleton instance.
    static void killInstance();

    ~PluginPoolManager() override;

    // Configuration

    /// Sets how many patches ahead to preload (1-5).
    ///
    /// @param patchesAhead Number of patches to preload ahead of the current position.
    void setPreloadRange(int patchesAhead);

    /// Gets the current preload range.
    int getPreloadRange() const { return preloadRange; }

    /// Sets the memory limit for the pool (optional, 0 = unlimited).
    ///
    /// @param bytes Maximum memory in bytes, or 0 for unlimited.
    void setMemoryLimit(size_t bytes);

    /// Gets estimated memory usage of the pool.
    ///
    /// @return Estimated memory usage in bytes.
    size_t getPoolMemoryUsage() const;

    // Setlist Management

    /// Clears all cached patches and plugin pool.
    void clear();

    /// Adds a patch's XML definition to the pool's knowledge.
    /// Call this for each patch in the setlist.
    ///
    /// @param patchIndex Index of the patch in the setlist.
    /// @param patchXml XML definition of the patch; ownership is transferred.
    void addPatchDefinition(int patchIndex, std::unique_ptr<XmlElement> patchXml);

    /// Removes a patch definition and its associated plugin requirements.
    /// Call this when a patch is deleted from the setlist.
    ///
    /// @param patchIndex Index of the patch to remove.
    void removePatchDefinition(int patchIndex);

    /// Gets the number of known patch definitions.
    int getNumPatches() const { return static_cast<int>(patchDefinitions.size()); }

    // Position & Switching

    /// Sets the current setlist position and triggers background preloading.
    /// This slides the loading window to keep prev/next patches ready.
    ///
    /// @param setlistIndex Index of the current patch in the setlist.
    void setCurrentPosition(int setlistIndex);

    /// Gets the current position.
    int getCurrentPosition() const { return currentPatchIndex.load(); }

    /// Checks if a patch is fully loaded and ready for instant switch.
    ///
    /// @param patchIndex Index of the patch to check.
    /// @return True if the patch is fully loaded.
    bool isPatchReady(int patchIndex) const;

    /// Gets loading progress for a patch (0.0 to 1.0).
    ///
    /// @param patchIndex Index of the patch to query.
    /// @return Loading progress from 0.0 to 1.0.
    float getPatchLoadProgress(int patchIndex) const;

    // Plugin Access

    /// Gets or creates a plugin instance from the pool.
    /// Returns nullptr if plugin couldn't be created.
    ///
    /// @param desc Plugin description identifying the plugin to get or create.
    /// @return The plugin instance, or nullptr if creation failed.
    AudioPluginInstance* getOrCreatePlugin(const PluginDescription& desc);

    /// Gets a plugin by its identifier string (from pool).
    ///
    /// @param identifier Plugin identifier string created by createPluginIdentifier.
    /// @return The plugin instance, or nullptr if not found in the pool.
    AudioPluginInstance* getPluginByIdentifier(const String& identifier);

    // Listeners

    /// Adds a listener for plugin pool events.
    void addListener(PluginPoolListener* listener);
    /// Removes a previously added listener.
    void removeListener(PluginPoolListener* listener);

  private:
    /// Private constructor for singleton pattern.
    PluginPoolManager();

    /// Static singleton instance pointer.
    static std::unique_ptr<PluginPoolManager> instance;

    // Thread implementation (background loading)

    /// Background thread entry point that processes the load queue.
    void run() override;

    /// Queue a patch for background loading.
    ///
    /// @param patchIndex Index of the patch to queue.
    void queuePatchLoad(int patchIndex);

    /// Load a single patch's plugins (called from background thread).
    ///
    /// @param patchIndex Index of the patch to load.
    void loadPatchPlugins(int patchIndex);

    /// Parse plugin descriptions from patch XML.
    ///
    /// @param patchXml XML element of the patch (Patch root or FILTERGRAPH).
    /// @return Vector of plugin descriptions extracted from the patch.
    std::vector<PluginDescription> extractPluginsFromPatch(const XmlElement* patchXml);

#if defined(PEDALBOARD3_TESTS)
  public:
    /// Test-only helper to exercise patch plugin extraction.
    ///
    /// @param patchXml XML element of the patch (Patch root or FILTERGRAPH).
    /// @return Vector of plugin descriptions extracted from the patch.
    static std::vector<PluginDescription> extractPluginsFromPatchForTest(const XmlElement* patchXml);

  private:
#endif

    /// Release plugins that are outside the current window.
    void releaseUnusedPlugins();

    /// Creates identifier string for a plugin description.
    ///
    /// @param desc Plugin description to identify.
    /// @return Identifier string in the form "format|name|uniqueId".
    static String createPluginIdentifier(const PluginDescription& desc);

    // Data

    /// Plugin pool - key is plugin identifier, value is pooled instance.
    std::map<String, std::unique_ptr<PooledPlugin>> pluginPool;

    /// Patch definitions (XML) - key is patch index.
    std::map<int, std::unique_ptr<XmlElement>> patchDefinitions;

    /// Which plugins each patch needs - key is patch index.
    std::map<int, std::vector<String>> patchPluginRequirements;

    /// Set of patches that are fully loaded.
    std::set<int> loadedPatches;

    /// Loading progress per patch (0.0 to 1.0).
    std::map<int, float> patchLoadProgress;

    /// Current patch index (atomic for thread safety).
    std::atomic<int> currentPatchIndex{0};

    /// Preload range (patches ahead to load).
    int preloadRange = 2;

    /// Memory limit (0 = unlimited).
    size_t memoryLimit = 0;

    /// Queue of patches to load.
    std::vector<int> loadQueue;

    /// Mutex for thread-safe access.
    mutable CriticalSection poolLock;

    /// Listeners for progress notifications.
    ListenerList<PluginPoolListener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginPoolManager)
};

#endif // PLUGINPOOLMANAGER_H_
