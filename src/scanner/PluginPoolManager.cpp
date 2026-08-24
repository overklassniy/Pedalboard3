// PluginPoolManager.cpp - Sliding window plugin pool for instant patch switching.
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

#include "PluginPoolManager.h"

#include "AudioSingletons.h"
#include "BypassableInstance.h"

#include <spdlog/spdlog.h>

/// Anonymous namespace containing helpers for extracting plugin descriptions
/// from patch XML, including nested rack and NAM processor states.
namespace {
/// Returns true if the plugin should be pooled (i.e. it is not an internal
/// plugin or an AudioUnit, which are handled separately).
///
/// @param desc Plugin description to check.
/// @return True if the plugin should be pooled.
bool shouldPoolPlugin(const PluginDescription& desc) {
    // Internal plugins (Audio I/O, etc.) and AudioUnits are not pooled yet.
    return desc.pluginFormatName != "Internal" && desc.pluginFormatName != "AudioUnit";
}

/// Returns true if the description refers to an internal SubGraph rack node.
/// Internal rack nodes are identified by the fileOrIdentifier field.
///
/// @param desc Plugin description to check.
/// @return True if the description refers to a SubGraph rack node.
bool isSubGraphPlugin(const PluginDescription& desc) {
    if (desc.pluginFormatName != "Internal")
        return false;

    return desc.fileOrIdentifier == "Internal:SubGraph";
}

/// Returns true if the description refers to the internal NAM (Neural Amp
/// Modeler) processor.
///
/// @param desc Plugin description to check.
/// @return True if the description refers to the NAM processor.
bool isNAMProcessor(const PluginDescription& desc) {
    if (desc.pluginFormatName != "Internal")
        return false;

    return desc.fileOrIdentifier == "NAM Loader";
}

void extractPluginsFromFilter(const XmlElement& filterElem, std::vector<PluginDescription>& result);
void extractPluginsFromNAMState(const XmlElement& filterElem, std::vector<PluginDescription>& result);
std::vector<PluginDescription> extractPluginsFromPatchImpl(const XmlElement* patchXml);

/// Extracts plugin descriptions from a SubGraph rack state embedded in a
/// FILTER element's STATE child. The state is base64-encoded XML with a RACK
/// root tag containing nested FILTER elements.
///
/// @param filterElem FILTER element whose STATE child holds the rack XML.
/// @param result Output vector appended with extracted plugin descriptions.
void extractPluginsFromRackState(const XmlElement& filterElem, std::vector<PluginDescription>& result) {
    auto* stateElem = filterElem.getChildByName("STATE");
    if (stateElem == nullptr)
        return;

    juce::MemoryBlock state;
    if (!state.fromBase64Encoding(stateElem->getAllSubText()))
        return;

    auto rackXml = juce::AudioProcessor::getXmlFromBinary(state.getData(), (int)state.getSize());
    if (!rackXml || !rackXml->hasTagName("RACK"))
        return;

    for (auto* rackFilter : rackXml->getChildWithTagNameIterator("FILTER"))
        extractPluginsFromFilter(*rackFilter, result);
}

/// Extracts plugin descriptions from a NAM processor's internal state.
///
/// The NAM processor stores its state as a binary stream: version, model path,
/// IR path, 9 parameters, effects-loop enabled flag, and a SubGraph state
/// block. The effects loop was introduced in version 2, so earlier states are
/// skipped. The SubGraph state block uses the same RACK XML format as
/// extractPluginsFromRackState.
///
/// @param filterElem FILTER element whose STATE child holds the NAM binary state.
/// @param result Output vector appended with extracted plugin descriptions.
void extractPluginsFromNAMState(const XmlElement& filterElem, std::vector<PluginDescription>& result) {
    auto* stateElem = filterElem.getChildByName("STATE");
    if (stateElem == nullptr)
        return;

    juce::MemoryBlock state;
    if (!state.fromBase64Encoding(stateElem->getAllSubText()))
        return;

    juce::MemoryInputStream stream(state, false);

    int version = stream.readInt();
    if (version < 2)
        return; // Effects loop was added in version 2

    // Skip model and IR paths
    stream.readString(); // model path
    stream.readString(); // ir path

    // Skip 9 parameters: inputGain, outputGain, noiseGateThreshold, bass, mid, treble
    for (int i = 0; i < 6; ++i)
        stream.readFloat();
    // toneStackEnabled, normalizeOutput, irEnabled
    for (int i = 0; i < 3; ++i)
        stream.readBool();

    stream.readBool();

    int fxLoopStateSize = stream.readInt();
    if (fxLoopStateSize <= 0)
        return;

    juce::MemoryBlock fxLoopState;
    fxLoopState.setSize(static_cast<size_t>(fxLoopStateSize));
    stream.read(fxLoopState.getData(), fxLoopStateSize);

    // Parse SubGraphProcessor state (same format as extractPluginsFromRackState)
    auto rackXml = juce::AudioProcessor::getXmlFromBinary(fxLoopState.getData(), (int)fxLoopState.getSize());
    if (!rackXml || !rackXml->hasTagName("RACK"))
        return;

    for (auto* rackFilter : rackXml->getChildWithTagNameIterator("FILTER"))
        extractPluginsFromFilter(*rackFilter, result);
}

/// Extracts plugin descriptions from a single FILTER element.
///
/// If the filter is a SubGraph or NAM processor, recursively extracts nested
/// plugins from their internal state. Otherwise, if the plugin is poolable,
/// adds it directly to the result vector.
///
/// @param filterElem FILTER element to extract plugin descriptions from.
/// @param result Output vector appended with extracted plugin descriptions.
void extractPluginsFromFilter(const XmlElement& filterElem, std::vector<PluginDescription>& result) {
    auto* descElem = filterElem.getChildByName("PLUGIN");
    if (descElem == nullptr)
        return;

    PluginDescription desc;
    if (!desc.loadFromXml(*descElem))
        return;

    if (isSubGraphPlugin(desc)) {
        extractPluginsFromRackState(filterElem, result);
        return;
    }

    if (isNAMProcessor(desc)) {
        extractPluginsFromNAMState(filterElem, result);
        return;
    }

    if (shouldPoolPlugin(desc))
        result.push_back(desc);
}

/// Extracts all plugin descriptions from a patch XML element.
///
/// Accepts either a Patch root element (containing a FILTERGRAPH child) or a
/// FILTERGRAPH element directly. Returns an empty vector if the XML is null or
/// contains no FILTER elements.
///
/// @param patchXml XML element of the patch (Patch root or FILTERGRAPH).
/// @return Vector of plugin descriptions extracted from the patch.
std::vector<PluginDescription> extractPluginsFromPatchImpl(const XmlElement* patchXml) {
    std::vector<PluginDescription> result;

    if (!patchXml)
        return result;

    const XmlElement* graphXml = patchXml;
    if (patchXml->hasTagName("Patch"))
        graphXml = patchXml->getChildByName("FILTERGRAPH");

    if (graphXml == nullptr)
        return result;

    // Look for FILTER elements (this is Pedalboard's XML format)
    for (auto* filterElem : graphXml->getChildWithTagNameIterator("FILTER"))
        extractPluginsFromFilter(*filterElem, result);

    return result;
}
} // namespace

std::unique_ptr<PluginPoolManager> PluginPoolManager::instance = nullptr;

PluginPoolManager& PluginPoolManager::getInstance() {
    if (!instance) {
        instance = std::unique_ptr<PluginPoolManager>(new PluginPoolManager());
    }
    return *instance;
}

void PluginPoolManager::killInstance() {
    if (instance) {
        instance.reset();
        spdlog::info("[PluginPoolManager] Singleton instance destroyed");
    }
}

PluginPoolManager::PluginPoolManager() : Thread("PluginPoolLoader") {
    spdlog::info("[PluginPoolManager] Initialized with preloadRange={}", preloadRange);
}

PluginPoolManager::~PluginPoolManager() {
    signalThreadShouldExit();
    notify();
    stopThread(5000);

    clear();

    spdlog::info("[PluginPoolManager] Destroyed");
}

void PluginPoolManager::setPreloadRange(int patchesAhead) {
    preloadRange = juce::jlimit(1, 5, patchesAhead);
    spdlog::info("[PluginPoolManager] Preload range set to {}", preloadRange);
}

void PluginPoolManager::setMemoryLimit(size_t bytes) {
    memoryLimit = bytes;
}

size_t PluginPoolManager::getPoolMemoryUsage() const {
    ScopedLock lock(poolLock);

    // Each plugin's actual memory footprint is not queried; a fixed average
    // of ~20 MB per instance is used instead.
    size_t estimate = 0;
    for (const auto& [key, pooled] : pluginPool) {
        if (pooled && pooled->instance) {
            estimate += 20 * 1024 * 1024;
        }
    }
    return estimate;
}

void PluginPoolManager::clear() {
    ScopedLock lock(poolLock);

    loadQueue.clear();

    pluginPool.clear();
    patchDefinitions.clear();
    patchPluginRequirements.clear();
    loadedPatches.clear();
    patchLoadProgress.clear();

    currentPatchIndex.store(0);

    spdlog::info("[PluginPoolManager] Pool cleared");
}

void PluginPoolManager::addPatchDefinition(int patchIndex, std::unique_ptr<XmlElement> patchXml) {
    if (!patchXml)
        return;

    ScopedLock lock(poolLock);

    patchDefinitions[patchIndex] = std::move(patchXml);

    auto plugins = extractPluginsFromPatch(patchDefinitions[patchIndex].get());
    std::vector<String> identifiers;
    for (const auto& desc : plugins) {
        identifiers.push_back(createPluginIdentifier(desc));
    }
    patchPluginRequirements[patchIndex] = std::move(identifiers);

    spdlog::debug("[PluginPoolManager] Added patch {} with {} plugins", patchIndex,
                  patchPluginRequirements[patchIndex].size());
}

void PluginPoolManager::setCurrentPosition(int setlistIndex) {
    int oldPosition = currentPatchIndex.exchange(setlistIndex);

    if (oldPosition == setlistIndex)
        return;

    spdlog::info("[PluginPoolManager] Position changed {} -> {}", oldPosition, setlistIndex);

    {
        ScopedLock lock(poolLock);

        // Clear load queue and reprioritize
        loadQueue.clear();

        // Queue patches in priority order:
        // 1. Current patch (if not loaded)
        // 2. Next patches (in order)
        // 3. Previous patch (for going back)

        if (!isPatchReady(setlistIndex))
            loadQueue.push_back(setlistIndex);

        for (int i = 1; i <= preloadRange; ++i) {
            int nextIndex = setlistIndex + i;
            if (patchDefinitions.count(nextIndex) > 0 && !isPatchReady(nextIndex))
                loadQueue.push_back(nextIndex);
        }

        // Previous patch (lower priority)
        int prevIndex = setlistIndex - 1;
        if (prevIndex >= 0 && patchDefinitions.count(prevIndex) > 0 && !isPatchReady(prevIndex))
            loadQueue.push_back(prevIndex);
    }

    if (!isThreadRunning())
        startThread();
    else
        notify();

    releaseUnusedPlugins();
}

bool PluginPoolManager::isPatchReady(int patchIndex) const {
    ScopedLock lock(poolLock);
    return loadedPatches.count(patchIndex) > 0;
}

float PluginPoolManager::getPatchLoadProgress(int patchIndex) const {
    ScopedLock lock(poolLock);

    if (loadedPatches.count(patchIndex) > 0)
        return 1.0f;

    auto it = patchLoadProgress.find(patchIndex);
    if (it != patchLoadProgress.end())
        return it->second;

    return 0.0f;
}

AudioPluginInstance* PluginPoolManager::getOrCreatePlugin(const PluginDescription& desc) {
    String identifier = createPluginIdentifier(desc);

    {
        ScopedLock lock(poolLock);

        auto it = pluginPool.find(identifier);
        if (it != pluginPool.end() && it->second && it->second->instance) {
            spdlog::debug("[PluginPoolManager] Returning cached plugin: {}", desc.name.toStdString());
            return it->second->instance.get();
        }
    }

    spdlog::info("[PluginPoolManager] Creating new plugin: {}", desc.name.toStdString());

    String errorMessage;
    auto newInstance =
        AudioPluginFormatManagerSingleton::getInstance().createPluginInstance(desc, 44100.0, 512, errorMessage);

    if (!newInstance) {
        spdlog::error("[PluginPoolManager] Failed to create plugin {}: {}", desc.name.toStdString(),
                      errorMessage.toStdString());
        return nullptr;
    }

    AudioProcessor::BusesLayout stereoLayout;
    stereoLayout.inputBuses.add(AudioChannelSet::stereo());
    stereoLayout.outputBuses.add(AudioChannelSet::stereo());
    if (newInstance->checkBusesLayoutSupported(stereoLayout))
        newInstance->setBusesLayout(stereoLayout);

    {
        ScopedLock lock(poolLock);

        auto pooled = std::make_unique<PooledPlugin>();
        pooled->instance = std::move(newInstance);
        pooled->description = desc;
        pooled->lastUsed = Time::getCurrentTime();

        AudioPluginInstance* result = pooled->instance.get();
        pluginPool[identifier] = std::move(pooled);
        return result;
    }
}

AudioPluginInstance* PluginPoolManager::getPluginByIdentifier(const String& identifier) {
    ScopedLock lock(poolLock);

    auto it = pluginPool.find(identifier);
    if (it != pluginPool.end() && it->second && it->second->instance)
        return it->second->instance.get();

    return nullptr;
}

void PluginPoolManager::addListener(PluginPoolListener* listener) {
    listeners.add(listener);
}

void PluginPoolManager::removeListener(PluginPoolListener* listener) {
    listeners.remove(listener);
}

void PluginPoolManager::run() {
    spdlog::info("[PluginPoolManager] Background loader thread started");

    while (!threadShouldExit()) {
        int patchToLoad = -1;

        {
            ScopedLock lock(poolLock);
            if (!loadQueue.empty()) {
                patchToLoad = loadQueue.front();
                loadQueue.erase(loadQueue.begin());
            }
        }

        if (patchToLoad >= 0) {
            loadPatchPlugins(patchToLoad);
        } else {
            wait(-1);
        }
    }

    spdlog::info("[PluginPoolManager] Background loader thread stopped");
}

void PluginPoolManager::queuePatchLoad(int patchIndex) {
    ScopedLock lock(poolLock);

    if (loadedPatches.count(patchIndex) == 0) {
        bool found = std::find(loadQueue.begin(), loadQueue.end(), patchIndex) != loadQueue.end();
        if (!found) {
            loadQueue.push_back(patchIndex);
            notify();
        }
    }
}

void PluginPoolManager::loadPatchPlugins(int patchIndex) {
    spdlog::info("[PluginPoolManager] Loading patch {}", patchIndex);

    std::vector<PluginDescription> plugins;

    {
        ScopedLock lock(poolLock);

        auto it = patchDefinitions.find(patchIndex);
        if (it == patchDefinitions.end()) {
            spdlog::warn("[PluginPoolManager] Patch {} not found in definitions", patchIndex);
            return;
        }

        plugins = extractPluginsFromPatch(it->second.get());
        patchLoadProgress[patchIndex] = 0.0f;
    }

    if (plugins.empty()) {
        ScopedLock lock(poolLock);
        loadedPatches.insert(patchIndex);
        patchLoadProgress[patchIndex] = 1.0f;
        MessageManager::callAsync(
            [this, patchIndex]() { listeners.call(&PluginPoolListener::patchReady, patchIndex); });
        return;
    }

    int loaded = 0;
    for (const auto& desc : plugins) {
        if (threadShouldExit())
            return;

        // Abort if the current position has moved far enough that this patch
        // is no longer within the preload window.
        int currentPos = currentPatchIndex.load();
        if (std::abs(patchIndex - currentPos) > preloadRange + 1) {
            spdlog::info("[PluginPoolManager] Aborting load of patch {} (too far from current {})", patchIndex,
                         currentPos);
            return;
        }

        getOrCreatePlugin(desc);
        loaded++;

        float progress = static_cast<float>(loaded) / static_cast<float>(plugins.size());
        {
            ScopedLock lock(poolLock);
            patchLoadProgress[patchIndex] = progress;
        }

        MessageManager::callAsync([this, patchIndex, progress]() {
            listeners.call(&PluginPoolListener::patchLoadingProgress, patchIndex, progress);
        });
    }

    {
        ScopedLock lock(poolLock);
        loadedPatches.insert(patchIndex);
        patchLoadProgress[patchIndex] = 1.0f;
    }

    spdlog::info("[PluginPoolManager] Patch {} fully loaded ({} plugins)", patchIndex, plugins.size());

    MessageManager::callAsync([this, patchIndex]() { listeners.call(&PluginPoolListener::patchReady, patchIndex); });
}

std::vector<PluginDescription> PluginPoolManager::extractPluginsFromPatch(const XmlElement* patchXml) {
    return extractPluginsFromPatchImpl(patchXml);
}

#if defined(PEDALBOARD3_TESTS)
std::vector<PluginDescription> PluginPoolManager::extractPluginsFromPatchForTest(const XmlElement* patchXml) {
    return extractPluginsFromPatchImpl(patchXml);
}
#endif

void PluginPoolManager::releaseUnusedPlugins() {
    ScopedLock lock(poolLock);

    int currentPos = currentPatchIndex.load();

    // Build set of plugins needed by patches in current window
    std::set<String> neededPlugins;

    for (int i = currentPos - 1; i <= currentPos + preloadRange; ++i) {
        if (i >= 0 && patchPluginRequirements.count(i) > 0) {
            for (const auto& id : patchPluginRequirements[i])
                neededPlugins.insert(id);
        }
    }

    std::vector<String> toRemove;
    for (const auto& [key, pooled] : pluginPool) {
        if (neededPlugins.count(key) == 0)
            toRemove.push_back(key);
    }

    for (const auto& key : toRemove) {
        spdlog::debug("[PluginPoolManager] Releasing unused plugin: {}", key.toStdString());
        pluginPool.erase(key);
    }

    // Also remove loaded status for patches outside window
    std::vector<int> patchesToUnload;
    for (int patch : loadedPatches) {
        if (patch < currentPos - 1 || patch > currentPos + preloadRange)
            patchesToUnload.push_back(patch);
    }
    for (int patch : patchesToUnload) {
        loadedPatches.erase(patch);
        patchLoadProgress.erase(patch);
    }

    if (!toRemove.empty()) {
        spdlog::info("[PluginPoolManager] Released {} unused plugins", toRemove.size());
    }
}

String PluginPoolManager::createPluginIdentifier(const PluginDescription& desc) {
    return desc.pluginFormatName + "|" + desc.name + "|" + String(desc.uniqueId);
}
