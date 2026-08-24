// SafePluginScanner.h - Safe plugin scanning with out-of-process isolation.
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

#include "PluginScannerClient.h"

#include <JuceHeader.h>

/// Provides safe plugin scanning with crash isolation and timeouts.
///
/// Uses the out-of-process scanner when available, falling back to
/// in-process scanning with timeout protection otherwise.
///
/// Note: This class wraps PluginDirectoryScanner rather than inheriting from it,
/// since JUCE's PluginDirectoryScanner methods aren't virtual.
class SafePluginScanner {
  public:
    /// Creates a safe plugin scanner.
    ///
    /// @param listToAddTo Receives discovered plugin descriptions.
    /// @param formatToScan Plugin format to scan for.
    /// @param directoriesToSearch Search paths for plugin files.
    /// @param searchRecursively Whether to traverse subdirectories.
    /// @param deadMansPedalFile Used by JUCE for crash tracking.
    /// @param useOutOfProcess Selects out-of-process scanning when true (the default).
    SafePluginScanner(juce::KnownPluginList& listToAddTo, juce::AudioPluginFormat& formatToScan,
                      juce::FileSearchPath directoriesToSearch, bool searchRecursively,
                      const juce::File& deadMansPedalFile, bool useOutOfProcess = true);

    /// Stops the scanner client if running.
    ~SafePluginScanner();

    /// Scans the next plugin file.
    ///
    /// When dontRescanIfAlreadyInList is true, plugins already in the list are
    /// skipped. nameOfPluginBeingScanned is set to the name of the plugin being
    /// scanned. Returns true if there are more plugins to scan.
    ///
    /// @param dontRescanIfAlreadyInList If true, skip plugins already in the list.
    /// @param nameOfPluginBeingScanned Set to the name of the plugin being scanned.
    /// @return True if there are more plugins to scan.
    bool scanNextFile(bool dontRescanIfAlreadyInList, juce::String& nameOfPluginBeingScanned);

    /// Get scan progress (0.0 to 1.0).
    ///
    /// @return Scan progress from 0.0 to 1.0.
    float getProgress() const;

    /// Get the next file that will be scanned.
    ///
    /// @return Path of the next plugin file to be scanned, or empty if none remain.
    juce::String getNextPluginFileThatWillBeScanned() const;

    /// Check if out-of-process scanning is being used.
    bool isUsingOutOfProcess() const { return useOutOfProcessScanning && scannerClient != nullptr; }

    /// Set timeout for individual plugin scans (ms).
    void setScanTimeout(int timeoutMs) { scanTimeoutMs = timeoutMs; }

  private:
    std::unique_ptr<juce::PluginDirectoryScanner> baseScanner;
    bool useOutOfProcessScanning;
    std::unique_ptr<PluginScannerClient> scannerClient;
    int scanTimeoutMs = 30000;

    juce::KnownPluginList& pluginList;
    juce::AudioPluginFormat& format;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SafePluginScanner)
};

/// Drop-in replacement for PluginListComponent with out-of-process scanning.
///
/// Uses SafePluginScanner internally for crash-safe plugin discovery.
class SafePluginListComponent : public juce::Component,
                                public juce::TableListBoxModel,
                                public juce::Button::Listener,
                                private juce::Timer {
  public:
    SafePluginListComponent(juce::AudioPluginFormatManager& formatManager, juce::KnownPluginList& listToRepresent,
                            const juce::File& deadMansPedalFile, juce::PropertiesFile* propertiesToUse = nullptr);

    /// Cancels any ongoing scan and destroys the component.
    ~SafePluginListComponent() override;

    // Component overrides
    void resized() override;
    void paint(juce::Graphics& g) override;

    // TableListBoxModel overrides
    int getNumRows() override;
    void paintRowBackground(juce::Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent&) override;
    void sortOrderChanged(int newSortColumnId, bool isForwards) override;

    // Button::Listener
    void buttonClicked(juce::Button* button) override;

    /// Start scanning for plugins.
    void startScan();

    /// Cancel an ongoing scan.
    void cancelScan();

    /// Check if a scan is in progress.
    bool isScanning() const { return scanning; }

  private:
    /// Periodically scans the next plugin file and updates the UI.
    void timerCallback() override;
    /// Refreshes the table from the current plugin list.
    void updateList();
    /// Resets the scan UI and releases the scanner after a scan completes.
    void scanFinished();

    juce::AudioPluginFormatManager& formatManager;
    juce::KnownPluginList& pluginList;
    juce::File deadMansPedal;

    std::unique_ptr<juce::TableListBox> table;
    std::unique_ptr<juce::TextButton> scanButton;
    std::unique_ptr<juce::TextButton> clearButton;
    std::unique_ptr<juce::TextButton> removeButton;
    std::unique_ptr<juce::Label> progressLabel;
    std::unique_ptr<juce::ProgressBar> progressBar;

    std::unique_ptr<SafePluginScanner> scanner;
    double scanProgress = 0.0;
    bool scanning = false;

    juce::StringArray pluginNames;
    juce::Array<int> sortedIndices;
    int sortColumnId = 1;
    bool sortForward = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SafePluginListComponent)
};
