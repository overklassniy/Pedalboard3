// SafePluginScanner.h - Safe plugin scanning with out-of-process isolation.
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
    /// Starts scanning the next format in the format manager.
    void startNextFormatScan();

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
    int currentFormatIndex = 0;

    juce::StringArray pluginNames;
    juce::Array<int> sortedIndices;
    int sortColumnId = 1;
    bool sortForward = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SafePluginListComponent)
};

/// Subclass of FileSearchPathListComponent that opens the folder in
/// the system file manager on double-click instead of showing a
/// "Change folder..." file chooser dialog.
class ScanPathListComponent : public juce::FileSearchPathListComponent {
  public:
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override {
        if (row >= 0 && row < getPath().getNumPaths()) {
            juce::File dir(getPath().getRawString(row));
            if (dir.exists())
                dir.startAsProcess();
        }
    }
};

/// Content component for the plugin scan path selection dialog.
///
/// Contains a ScanPathListComponent and Scan/Cancel buttons. When
/// either button is clicked, the parent DialogWindow's modal state is
/// ended with the corresponding result code (1 for Scan, 0 for
/// Cancel). The dialog window itself uses a native OS title bar with
/// a close button, so no custom close button is needed here.
class ScanPathDialogContent : public juce::Component,
                              public juce::Button::Listener {
  public:
    /// Creates the dialog content with the given initial search path.
    ///
    /// @param initialPath The search path to pre-populate the path list with.
    ScanPathDialogContent(const juce::FileSearchPath& initialPath) {
        pathList.setPath(initialPath);
        addAndMakeVisible(pathList);

        scanButton.setButtonText("Scan");
        scanButton.addListener(this);
        scanButton.addShortcut(juce::KeyPress(juce::KeyPress::returnKey));
        addAndMakeVisible(scanButton);

        cancelButton.setButtonText("Cancel");
        cancelButton.addListener(this);
        cancelButton.addShortcut(juce::KeyPress(juce::KeyPress::escapeKey));
        addAndMakeVisible(cancelButton);
    }

    /// Lays out the path list and buttons.
    void resized() override {
        auto bounds = getLocalBounds().reduced(8);

        auto buttonRow = bounds.removeFromBottom(28);
        scanButton.setBounds(buttonRow.removeFromRight(80).withTrimmedRight(8));
        cancelButton.setBounds(buttonRow.removeFromRight(80).withTrimmedRight(8));

        pathList.setBounds(bounds);
    }

    /// Returns the current search path shown in the path list.
    const juce::FileSearchPath& getPath() const { return pathList.getPath(); }

    /// Ends the parent DialogWindow's modal state with the clicked
    /// button's result code.
    void buttonClicked(juce::Button* button) override {
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(button == &scanButton ? 1 : 0);
    }

  private:
    ScanPathListComponent pathList;
    juce::TextButton scanButton;
    juce::TextButton cancelButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScanPathDialogContent)
};

/// Subclass of juce::PluginListComponent that adds a single unified
/// "Scan for all plugins" menu entry alongside the standard JUCE options.
///
/// Inherits all JUCE functionality (path selection dialog, table display,
/// theme-aware colours, remove/clear options). The options button shows
/// the standard JUCE menu plus a unified scan entry that scans all
/// registered formats sequentially.
class UnifiedPluginListComponent : public juce::PluginListComponent,
                                   private juce::Timer {
  public:
    UnifiedPluginListComponent(juce::AudioPluginFormatManager& formatManager,
                               juce::KnownPluginList& listToRepresent,
                               const juce::File& deadMansPedalFile,
                               juce::PropertiesFile* propertiesToUse)
        : juce::PluginListComponent(formatManager, listToRepresent, deadMansPedalFile,
                                    propertiesToUse, true),
          storedFormatManager(formatManager),
          storedPluginList(listToRepresent),
          storedProperties(propertiesToUse) {
        // Replace the default options button handler with our unified menu.
        getOptionsButton().onClick = [this] { showUnifiedOptionsMenu(); };
    }

  private:
    /// Shows the options menu with a single unified scan entry instead
    /// of per-format scan entries.
    void showUnifiedOptionsMenu() {
        juce::PopupMenu menu;

        menu.addItem(juce::PopupMenu::Item("Clear list")
                        .setAction([this] { storedPluginList.clear(); }));

        menu.addSeparator();

        menu.addItem(juce::PopupMenu::Item("Remove selected plug-in from list")
                        .setEnabled(getTableListBox().getNumSelectedRows() > 0)
                        .setAction([this] { removeSelectedPlugins(); }));

        menu.addItem(juce::PopupMenu::Item("Remove any plug-ins whose files no longer exist")
                        .setAction([this] {
                            auto types = storedPluginList.getTypes();
                            for (auto pd : types) {
                                if (!juce::File::isAbsolutePath(pd.fileOrIdentifier) ||
                                    !juce::File(pd.fileOrIdentifier).existsAsFile())
                                    storedPluginList.removeType(pd);
                            }
                        }));

        menu.addSeparator();

        auto selectedRow = getTableListBox().getSelectedRow();
        menu.addItem(juce::PopupMenu::Item("Show folder containing selected plug-in")
                        .setEnabled(selectedRow >= 0)
                        .setAction([this, selectedRow] {
                            if (selectedRow >= 0) {
                                auto types = storedPluginList.getTypes();
                                if (selectedRow < types.size()) {
                                    auto pd = types[selectedRow];
                                    if (juce::File::isAbsolutePath(pd.fileOrIdentifier)) {
                                        juce::File(pd.fileOrIdentifier).getParentDirectory().startAsProcess();
                                    }
                                }
                            }
                        }));

        menu.addSeparator();

        // Single unified scan entry for all formats.
        menu.addItem(juce::PopupMenu::Item("Scan for new or updated VST/VST3/LADSPA")
                        .setAction([this] { startUnifiedScan(); }));

        menu.showMenuAsync(juce::PopupMenu::Options()
                               .withDeletionCheck(*this)
                               .withTargetComponent(getOptionsButton()));
    }

    /// Starts scanning all registered formats sequentially.
    /// Shows a single path selection dialog, then scans all formats
    /// using the selected path.
    void startUnifiedScan() {
        formatsToScan.clear();

        for (int i = 0; i < storedFormatManager.getNumFormats(); ++i) {
            auto* format = storedFormatManager.getFormat(i);
            if (format->canScanForPlugins())
                formatsToScan.add(format);
        }

        if (formatsToScan.isEmpty())
            return;

        // Show a single path selection dialog using the first format's
        // default search locations as the initial path.
        auto* firstFormat = formatsToScan[0];
        auto defaultPath = firstFormat->getDefaultLocationsToSearch();

        // Load the saved path directly. If the key exists (even if empty),
        // use the saved value so that an explicitly empty path is
        // respected instead of falling back to defaults.
        if (storedProperties) {
            auto key = "lastPluginScanPath_" + firstFormat->getName();
            if (storedProperties->containsKey(key)) {
                auto saved = juce::FileSearchPath(storedProperties->getValue(key));
                defaultPath = saved;
            }
        }

        auto* content = new ScanPathDialogContent(defaultPath);
        content->setSize(500, 300);

        juce::DialogWindow::LaunchOptions options;
        options.dialogTitle = "Plugin Scanning";
        options.dialogBackgroundColour = juce::Colour(0xffeeece1);
        options.content.setOwned(content);
        options.componentToCentreAround = this;
        options.escapeKeyTriggersCloseButton = true;
        options.useNativeTitleBar = true;
        options.resizable = true;
        options.useBottomRightCornerResizer = false;

        // Use create() instead of launchAsync() so we can attach our
        // own modal callback. launchAsync() calls enterModalState
        // internally with a null callback, which would prevent us from
        // attaching one. The DialogWindow owns the content and
        // auto-deletes when modal state ends; the content is still
        // alive when the callback runs, so we can read the path from
        // it before the DialogWindow deletes it.
        auto* dw = options.create();
        dw->enterModalState(true, juce::ModalCallbackFunction::create(
            [this, content](int result) {
                auto searchPath = content->getPath();

                // Always save the path for all formats, even if the user
                // cancelled, so folder add/remove edits persist.
                // Save directly instead of using setLastSearchPath, which
                // removes the key when the path is empty — that would
                // cause getLastSearchPath to fall back to defaults.
                if (storedProperties) {
                    for (auto* fmt : formatsToScan) {
                        auto key = "lastPluginScanPath_" + fmt->getName();
                        storedProperties->setValue(key, searchPath.toString());
                    }
                    storedProperties->saveIfNeeded();
                }

                if (result == 0)
                    return;

                // Find plugin files for each format in the selected path
                // and collect them so we can scan without showing the
                // path dialog again. Use recursive search so plugins
                // in subfolders are found.
                pendingScans.clear();
                for (auto* fmt : formatsToScan) {
                    auto files = fmt->searchPathsForPlugins(searchPath, true, false);
                    if (!files.isEmpty())
                        pendingScans.add({fmt, files});
                }

                if (pendingScans.isEmpty())
                    return;

                currentFormatScanIndex = 0;
                scanNextFormat();
            }), true);
    }

    /// Scans the next format in the queue using the pre-collected file
    /// list, so no path dialog is shown.
    void scanNextFormat() {
        if (currentFormatScanIndex >= pendingScans.size()) {
            stopTimer();
            return;
        }

        auto entry = pendingScans[currentFormatScanIndex];
        scanFor(*entry.format, entry.files);
        startTimer(50);
    }

    /// Polls isScanning() and advances to the next format when the
    /// current scan completes.
    void timerCallback() override {
        if (!isScanning()) {
            stopTimer();
            ++currentFormatScanIndex;
            scanNextFormat();
        }
    }

    /// A format plus the files to scan for that format.
    struct FormatScanEntry {
        juce::AudioPluginFormat* format;
        juce::StringArray files;
    };

    juce::AudioPluginFormatManager& storedFormatManager;
    juce::KnownPluginList& storedPluginList;
    juce::PropertiesFile* storedProperties;
    juce::Array<juce::AudioPluginFormat*> formatsToScan;
    juce::Array<FormatScanEntry> pendingScans;
    int currentFormatScanIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnifiedPluginListComponent)
};
