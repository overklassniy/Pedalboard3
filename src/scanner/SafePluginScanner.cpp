// SafePluginScanner.cpp - Safe plugin scanning with out-of-process isolation.
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

#include "SafePluginScanner.h"

#include "CrashProtection.h"
#include "PluginBlacklist.h"

#include <spdlog/spdlog.h>

SafePluginScanner::SafePluginScanner(juce::KnownPluginList& listToAddTo, juce::AudioPluginFormat& formatToScan,
                                     juce::FileSearchPath directoriesToSearch, bool searchRecursively,
                                     const juce::File& deadMansPedalFile, bool useOutOfProcess)
    : useOutOfProcessScanning(useOutOfProcess), pluginList(listToAddTo), format(formatToScan) {
    baseScanner = std::make_unique<juce::PluginDirectoryScanner>(listToAddTo, formatToScan, directoriesToSearch,
                                                                 searchRecursively, deadMansPedalFile);

    if (useOutOfProcessScanning) {
        auto scannerExe = PluginScannerClient::getScannerExecutable();
        if (scannerExe.existsAsFile()) {
            scannerClient = std::make_unique<PluginScannerClient>();
            spdlog::info("[SafePluginScanner] Using out-of-process scanning");
        } else {
            spdlog::warn("[SafePluginScanner] Scanner executable not found at {}, falling back to in-process scanning",
                         scannerExe.getFullPathName().toStdString());
            useOutOfProcessScanning = false;
        }
    }
}

SafePluginScanner::~SafePluginScanner() {
    if (scannerClient)
        scannerClient->stopScanner();
}

juce::String SafePluginScanner::getNextPluginFileThatWillBeScanned() const {
    if (baseScanner)
        return baseScanner->getNextPluginFileThatWillBeScanned();
    return {};
}

float SafePluginScanner::getProgress() const {
    if (baseScanner)
        return baseScanner->getProgress();
    return 1.0f;
}

bool SafePluginScanner::scanNextFile(bool dontRescanIfAlreadyInList, juce::String& nameOfPluginBeingScanned) {
    if (!baseScanner)
        return false;

    juce::String nextFile = baseScanner->getNextPluginFileThatWillBeScanned();

    if (nextFile.isEmpty())
        return false;

    nameOfPluginBeingScanned = juce::File(nextFile).getFileName();

    if (PluginBlacklist::getInstance().isBlacklisted(nextFile)) {
        spdlog::debug("[SafePluginScanner] Skipping blacklisted plugin: {}", nextFile.toStdString());
        // Let the base scanner handle advancing past the blacklisted file.
        return baseScanner->scanNextFile(dontRescanIfAlreadyInList, nameOfPluginBeingScanned);
    }

    if (useOutOfProcessScanning && scannerClient) {
        juce::OwnedArray<juce::PluginDescription> results;

        bool scanSuccess = scannerClient->scanPlugin(nextFile, format.getName(), results);

        if (scanSuccess) {
            for (auto* desc : results) {
                pluginList.addType(*desc);
            }
            spdlog::debug("[SafePluginScanner] Out-of-process scan found {} plugin(s) in {}", results.size(),
                          nextFile.toStdString());
        } else {
            spdlog::warn("[SafePluginScanner] Out-of-process scan failed for: {}", nextFile.toStdString());
        }

        // The base scanner may rescan, but the plugin is already in the list
        // so the rescan is fast.
        juce::String dummy;
        return baseScanner->scanNextFile(true, dummy);
    } else {
        struct ScanAttemptState {
            std::atomic<bool> success{false};
            juce::String scannedName;
        };

        auto state = std::make_shared<ScanAttemptState>();
        auto* scanner = baseScanner.get();
        const bool dontRescan = dontRescanIfAlreadyInList;

        auto result = CrashProtection::getInstance().executeWithProtectionAndTimeout(
            [state, scanner, dontRescan]() {
                juce::String scannedName;
                const bool ok = scanner->scanNextFile(dontRescan, scannedName);
                state->success.store(ok, std::memory_order_release);
                state->scannedName = scannedName;
            },
            "Plugin Scan: " + nameOfPluginBeingScanned, scanTimeoutMs, nextFile);

        if (result == TimedOperationResult::Timeout) {
            spdlog::warn("[SafePluginScanner] Scan timed out, plugin blacklisted: {}", nextFile.toStdString());
            // The timeout handler already blacklisted it, check if more files remain
            return !baseScanner->getNextPluginFileThatWillBeScanned().isEmpty();
        } else if (result == TimedOperationResult::Exception) {
            spdlog::error("[SafePluginScanner] Scan threw exception: {}", nextFile.toStdString());
            return !baseScanner->getNextPluginFileThatWillBeScanned().isEmpty();
        }

        nameOfPluginBeingScanned = state->scannedName;
        return state->success.load(std::memory_order_acquire);
    }
}

SafePluginListComponent::SafePluginListComponent(juce::AudioPluginFormatManager& fm,
                                                 juce::KnownPluginList& listToRepresent,
                                                 const juce::File& deadMansPedalFile,
                                                 juce::PropertiesFile* /*propertiesToUse*/)
    : formatManager(fm), pluginList(listToRepresent), deadMansPedal(deadMansPedalFile) {
    table = std::make_unique<juce::TableListBox>("plugins", this);
    table->getHeader().addColumn("Name", 1, 200, 100, 400);
    table->getHeader().addColumn("Type", 2, 80, 50, 100);
    table->getHeader().addColumn("Category", 3, 100, 50, 150);
    table->getHeader().addColumn("Manufacturer", 4, 150, 100, 250);
    addAndMakeVisible(table.get());

    scanButton = std::make_unique<juce::TextButton>("Scan for new plugins...");
    scanButton->addListener(this);
    addAndMakeVisible(scanButton.get());

    clearButton = std::make_unique<juce::TextButton>("Clear list");
    clearButton->addListener(this);
    addAndMakeVisible(clearButton.get());

    removeButton = std::make_unique<juce::TextButton>("Remove selected");
    removeButton->addListener(this);
    addAndMakeVisible(removeButton.get());

    progressLabel = std::make_unique<juce::Label>("progress", "");
    progressLabel->setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(progressLabel.get());

    progressBar = std::make_unique<juce::ProgressBar>(scanProgress);
    progressBar->setVisible(false);
    addAndMakeVisible(progressBar.get());

    updateList();
}

SafePluginListComponent::~SafePluginListComponent() {
    cancelScan();
}

void SafePluginListComponent::resized() {
    auto bounds = getLocalBounds().reduced(4);

    auto buttonArea = bounds.removeFromBottom(30);
    scanButton->setBounds(buttonArea.removeFromLeft(150));
    buttonArea.removeFromLeft(8);
    clearButton->setBounds(buttonArea.removeFromLeft(80));
    buttonArea.removeFromLeft(8);
    removeButton->setBounds(buttonArea.removeFromLeft(100));

    bounds.removeFromBottom(4);

    auto progressArea = bounds.removeFromBottom(24);
    progressLabel->setBounds(progressArea.removeFromLeft(200));
    progressBar->setBounds(progressArea);

    bounds.removeFromBottom(4);
    table->setBounds(bounds);
}

void SafePluginListComponent::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

int SafePluginListComponent::getNumRows() {
    return pluginList.getNumTypes();
}

void SafePluginListComponent::paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/,
                                                 bool rowIsSelected) {
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
    else if (rowNumber % 2)
        g.fillAll(juce::Colour(0xffeeeeee));
}

void SafePluginListComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height,
                                        bool /*rowIsSelected*/) {
    if (rowNumber >= pluginList.getNumTypes())
        return;

    auto types = pluginList.getTypes();
    if (rowNumber >= types.size())
        return;

    const auto& desc = types[static_cast<size_t>(rowNumber)];

    g.setColour(juce::Colours::black);
    g.setFont(juce::Font(juce::FontOptions().withHeight(13.0f)));

    juce::String text;
    switch (columnId) {
    case 1:
        text = desc.name;
        break;
    case 2:
        text = desc.pluginFormatName;
        break;
    case 3:
        text = desc.category;
        break;
    case 4:
        text = desc.manufacturerName;
        break;
    }

    g.drawText(text, 4, 0, width - 8, height, juce::Justification::centredLeft, true);
}

void SafePluginListComponent::cellClicked(int rowNumber, int /*columnId*/, const juce::MouseEvent&) {
    table->selectRow(rowNumber);
}

void SafePluginListComponent::sortOrderChanged(int newSortColumnId, bool isForwards) {
    sortColumnId = newSortColumnId;
    sortForward = isForwards;
    updateList();
}

void SafePluginListComponent::buttonClicked(juce::Button* button) {
    if (button == scanButton.get()) {
        if (scanning)
            cancelScan();
        else
            startScan();
    } else if (button == clearButton.get()) {
        pluginList.clear();
        updateList();
    } else if (button == removeButton.get()) {
        auto selected = table->getSelectedRow();
        if (selected >= 0 && selected < pluginList.getNumTypes()) {
            auto types = pluginList.getTypes();
            pluginList.removeType(types[static_cast<size_t>(selected)]);
            updateList();
        }
    }
}

void SafePluginListComponent::startScan() {
    if (scanning)
        return;

    scanning = true;
    scanButton->setButtonText("Cancel scan");
    progressBar->setVisible(true);
    scanProgress = 0.0;
    progressLabel->setText("Starting scan...", juce::dontSendNotification);

    for (int i = 0; i < formatManager.getNumFormats(); ++i) {
        auto* format = formatManager.getFormat(i);
        if (format->getName() == "VST3") {
            auto searchPaths = format->getDefaultLocationsToSearch();
            scanner = std::make_unique<SafePluginScanner>(pluginList, *format, searchPaths, true, deadMansPedal, true);
            break;
        }
    }

    if (scanner)
        startTimer(100);
    else
        scanFinished();
}

void SafePluginListComponent::cancelScan() {
    if (!scanning)
        return;

    stopTimer();
    scanner.reset();
    scanFinished();
}

void SafePluginListComponent::timerCallback() {
    if (!scanner) {
        scanFinished();
        return;
    }

    juce::String pluginName;
    bool hasMore = scanner->scanNextFile(true, pluginName);

    scanProgress = static_cast<double>(scanner->getProgress());
    progressLabel->setText("Scanning: " + pluginName, juce::dontSendNotification);

    if (!hasMore) {
        scanFinished();
    }

    updateList();
}

void SafePluginListComponent::updateList() {
    table->updateContent();
    table->repaint();
}

void SafePluginListComponent::scanFinished() {
    scanning = false;
    scanner.reset();
    stopTimer();

    scanButton->setButtonText("Scan for new plugins...");
    progressBar->setVisible(false);
    progressLabel->setText("Scan complete. Found " + juce::String(pluginList.getNumTypes()) + " plugins.",
                           juce::dontSendNotification);

    updateList();
}
