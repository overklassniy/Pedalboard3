// PluginScannerClient.h - Host-side client for the out-of-process plugin scanner.
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

#pragma once

#include "PluginScannerIPC.h"

#include <JuceHeader.h>

/// Manages communication with the out-of-process plugin scanner.
///
/// Launches the scanner process, sends scan requests, and handles responses.
/// If the scanner crashes, it's automatically restarted for the next scan.
class PluginScannerClient {
  public:
    PluginScannerClient();
    ~PluginScannerClient();

    /// Scans a plugin file using the out-of-process scanner.
    ///
    /// @param pluginPath Full path to the plugin file.
    /// @param formatName Plugin format name (e.g. "VST3").
    /// @param results Output array filled with the discovered plugin descriptions.
    /// @return True if the scan completed successfully.
    bool scanPlugin(const juce::String& pluginPath, const juce::String& formatName,
                    juce::OwnedArray<juce::PluginDescription>& results);

    /// Check if the scanner process is currently running.
    ///
    /// @return True if the scanner process is running.
    bool isScannerRunning() const;

    /// Explicitly start the scanner process.
    /// Usually called automatically by scanPlugin().
    ///
    /// @return True if the scanner started successfully.
    bool startScanner();

    /// Stop the scanner process.
    void stopScanner();

    /// Get the path to the scanner executable.
    ///
    /// @return File object pointing to the scanner executable.
    static juce::File getScannerExecutable();

    /// Listener interface for scan progress updates.
    class Listener {
      public:
        virtual ~Listener() = default;
        /// Called when the scanner process has started and is ready.
        virtual void scannerStarted() {}
        /// Called when the scanner process has stopped.
        virtual void scannerStopped() {}
        /// Called when a scan of a specific plugin is about to begin.
        ///
        /// @param pluginPath Path of the plugin about to be scanned.
        virtual void scanProgress(const juce::String& pluginPath) {}
        /// Called when a scan of a specific plugin has finished. success
        /// indicates whether the scan succeeded.
        ///
        /// @param pluginPath Path of the plugin that was scanned.
        /// @param success True if the scan succeeded.
        virtual void scanComplete(const juce::String& pluginPath, bool success) {}
        /// Called when the scanner process crashed. lastPlugin is the plugin
        /// that was being scanned at the time of the crash.
        ///
        /// @param lastPlugin Path of the plugin being scanned when the crash occurred.
        virtual void scannerCrashed(const juce::String& lastPlugin) {}
    };

    /// Adds a listener for scanner events (scan progress, completion, crashes).
    void addListener(Listener* listener);
    /// Removes a previously added listener.
    void removeListener(Listener* listener);

  private:
    /// Starts the scanner process if it is not already running.
    ///
    /// @return True if the scanner is running after this call.
    bool ensureScannerRunning();
    /// Sends a scan request to the scanner process.
    ///
    /// @param request Scan request to send.
    /// @return True if the request was sent successfully.
    bool sendRequest(const PluginScannerIPC::ScanRequest& request);
    /// Waits for a scan response from the scanner process, up to timeoutMs.
    ///
    /// @param response Output parameter filled with the received scan response.
    /// @param timeoutMs Maximum time to wait in milliseconds.
    /// @return True if a response was received within the timeout.
    bool waitForResponse(PluginScannerIPC::ScanResponse& response, int timeoutMs);
    /// Handles a scanner crash by blacklisting the current plugin and
    /// cleaning up the scanner process.
    void handleScannerCrash();

    // Platform-specific handles stored as void* to avoid Windows header in header file
    void* pipeHandle = nullptr;
    void* scannerProcess = nullptr;

    juce::String lastScannedPlugin;
    juce::ListenerList<Listener> listeners;
    juce::CriticalSection scanLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginScannerClient)
};
