// PluginScannerClient.h - Host-side client for the out-of-process plugin scanner.
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

#include "PluginScannerIPC.h"

#include <JuceHeader.h>

/// Manages communication with the out-of-process plugin scanner.
///
/// Launches the scanner process, sends scan requests, and handles responses.
/// If the scanner crashes, it's automatically restarted for the next scan.
class PluginScannerClient
{
  public:
    PluginScannerClient();
    ~PluginScannerClient();

    /// Scan a plugin file using the out-of-process scanner.
    /// @param pluginPath Full path to the plugin file
    /// @param formatName Plugin format name (e.g., "VST3")
    /// @param results Output array for found plugin descriptions
    /// @return true if scan completed successfully
    bool scanPlugin(const juce::String& pluginPath, const juce::String& formatName,
                    juce::OwnedArray<juce::PluginDescription>& results);

    /// Check if the scanner process is currently running.
    bool isScannerRunning() const;

    /// Explicitly start the scanner process.
    /// Usually called automatically by scanPlugin().
    bool startScanner();

    /// Stop the scanner process.
    void stopScanner();

    /// Get the path to the scanner executable.
    static juce::File getScannerExecutable();

    /// Listener interface for scan progress updates.
    class Listener
    {
      public:
        virtual ~Listener() = default;
        virtual void scannerStarted() {}
        virtual void scannerStopped() {}
        virtual void scanProgress(const juce::String& pluginPath) {}
        virtual void scanComplete(const juce::String& pluginPath, bool success) {}
        virtual void scannerCrashed(const juce::String& lastPlugin) {}
    };

    /// Adds a listener for scanner events (scan progress, completion, crashes).
    void addListener(Listener* listener);
    /// Removes a previously added listener.
    void removeListener(Listener* listener);

  private:
    bool ensureScannerRunning();
    bool sendRequest(const PluginScannerIPC::ScanRequest& request);
    bool waitForResponse(PluginScannerIPC::ScanResponse& response, int timeoutMs);
    void handleScannerCrash();

    // Platform-specific handles stored as void* to avoid Windows header in header file
    void* pipeHandle = nullptr;
    void* scannerProcess = nullptr;

    juce::String lastScannedPlugin;
    juce::ListenerList<Listener> listeners;
    juce::CriticalSection scanLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginScannerClient)
};

