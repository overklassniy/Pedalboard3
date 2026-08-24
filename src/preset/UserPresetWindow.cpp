// UserPresetWindow.cpp - User preset window UI.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2012 Niall Moody.
//
// Modified for Pedalboard3 from the original Pedalboard2 source.
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

#include "UserPresetWindow.h"

UserPresetWindow::UserPresetWindow(KnownPluginList* knownPlugins) : knownPluginList(knownPlugins) {
    presetList = std::make_unique<TreeView>();
    addAndMakeVisible(*presetList);
    presetList->setName("presetList");

    copyButton = std::make_unique<TextButton>("copyButton");
    addAndMakeVisible(*copyButton);
    copyButton->setTooltip("Duplicate the selected preset");
    copyButton->setButtonText("Copy");
    copyButton->addListener(this);

    removeButton = std::make_unique<TextButton>("removeButton");
    addAndMakeVisible(*removeButton);
    removeButton->setTooltip("Delete the selected preset");
    removeButton->setButtonText("Remove");
    removeButton->addListener(this);

    importButton = std::make_unique<TextButton>("importButton");
    addAndMakeVisible(*importButton);
    importButton->setTooltip("Import a preset from an .fxp file");
    importButton->setButtonText("Import...");
    importButton->addListener(this);

    exportButton = std::make_unique<TextButton>("exportButton");
    addAndMakeVisible(*exportButton);
    exportButton->setTooltip("Export the selected preset to an .fxp file");
    exportButton->setButtonText("Export...");
    exportButton->addListener(this);

    renameButton = std::make_unique<TextButton>("renameButton");
    addAndMakeVisible(*renameButton);
    renameButton->setTooltip("Rename the selected preset");
    renameButton->setButtonText("Rename");
    renameButton->addListener(this);

    presetList->setColour(TreeView::backgroundColourId, Colours::white);
    presetList->setRootItem(&treeRoot);
    presetList->setRootItemVisible(false);

    setSize(600, 400);
}

UserPresetWindow::~UserPresetWindow() {
    presetList->setRootItem(nullptr);
}

void UserPresetWindow::paint(Graphics& g) {
    g.fillAll(Colour(0xffeeece1));

    g.setColour(Colour(0x40000000));
    g.fillRect(7, 7, getWidth() - 104, getHeight() - 13);
}

void UserPresetWindow::resized() {
    presetList->setBounds(8, 8, getWidth() - 106, getHeight() - 15);
    renameButton->setBounds(getWidth() - 90, 8, 82, 24);
    copyButton->setBounds(getWidth() - 90, 40, 82, 24);
    removeButton->setBounds(getWidth() - 90, 72, 82, 24);
    importButton->setBounds(getWidth() - 90, 104, 82, 24);
    exportButton->setBounds(getWidth() - 90, 136, 82, 24);
}

void UserPresetWindow::buttonClicked(Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == copyButton.get()) {
        PresetItem* selected = dynamic_cast<PresetItem*>(presetList->getSelectedItem(0));

        if (selected) {
            AlertWindow win("Copy Preset", "Enter a name for the duplicate preset:", MessageBoxIconType::NoIcon);

            win.addTextEditor("presetName", "");

            win.addButton("Cancel", 0);
            win.addButton("Ok", 1, KeyPress(KeyPress::returnKey));

            if (win.runModalLoop()) {
                String tempstr;
                File srcPreset = selected->getFile();

                tempstr = win.getTextEditorContents("presetName");
                tempstr << ".fxp";

                srcPreset.copyFileTo(srcPreset.getParentDirectory().getChildFile(tempstr));

                // Re-open the parent item to refresh its sub-items, since
                // treeHasChanged() does not reliably rebuild the tree.
                selected->getParentItem()->itemOpennessChanged(true);
                presetList->repaint();
            }
        }
    } else if (buttonThatWasClicked == removeButton.get()) {
        PresetItem* selected = dynamic_cast<PresetItem*>(presetList->getSelectedItem(0));

        if (selected) {
            File parentDir = selected->getFile().getParentDirectory();

            if (!selected->getFile().deleteFile()) {
                AlertWindow::showMessageBoxAsync(
                    MessageBoxIconType::WarningIcon, "Preset Deletion Error",
                    "Could not delete preset from the filesystem. Check your permissions.");
            } else {
                // If that plugin's directory is now empty, delete it too.
                if (!parentDir.getNumberOfChildFiles(File::findFilesAndDirectories))
                    parentDir.deleteFile();

                treeRoot.itemOpennessChanged(true);
                presetList->repaint();
            }
        }
    } else if (buttonThatWasClicked == importButton.get()) {
        StringArray plugins;

        for (int i = 0; i < knownPluginList->getNumTypes(); ++i)
            plugins.add(knownPluginList->getType(i)->name);

        // First get the user to select which plugin the preset is for.
        AlertWindow whichPlugin("Import Preset",
                                "Select which plugin this preset is intended for:", MessageBoxIconType::NoIcon);

        whichPlugin.addComboBox("plugins", plugins);
        whichPlugin.addButton("Cancel", 0);
        whichPlugin.addButton("Ok", 1, KeyPress(KeyPress::returnKey));

        if (whichPlugin.runModalLoop()) {
            FileChooser phil("Import preset", File(), "*.fxp");

            if (phil.browseForFileToOpen()) {
                String pluginName = whichPlugin.getComboBoxComponent("plugins")->getText();
                File presetDir = File::getSpecialLocation(File::userApplicationDataDirectory)
                                     .getChildFile("Pedalboard3")
                                     .getChildFile("presets");
                File pluginDir = presetDir.getChildFile(pluginName);
                File srcPreset = phil.getResult();

                if (!pluginDir.exists()) {
                    if (!pluginDir.createDirectory()) {
                        AlertWindow::showMessageBoxAsync(
                            MessageBoxIconType::WarningIcon, "Preset Import Error",
                            "Could not create a directory for this plugin. Check your permissions.");
                    }

                    phil.getResult().copyFileTo(pluginDir.getChildFile(srcPreset.getFileName()));

                    treeRoot.itemOpennessChanged(true);
                    presetList->repaint();
                }
            }
        }
    } else if (buttonThatWasClicked == exportButton.get()) {
        PresetItem* selected = dynamic_cast<PresetItem*>(presetList->getSelectedItem(0));

        if (selected) {
            FileChooser phil("Export preset", File(), "*.fxp");

            if (phil.browseForFileToSave(true))
                selected->getFile().copyFileTo(phil.getResult().withFileExtension("fxp"));
        }
    } else if (buttonThatWasClicked == renameButton.get()) {
        PresetItem* selected = dynamic_cast<PresetItem*>(presetList->getSelectedItem(0));

        if (selected) {
            AlertWindow win("Rename Preset", "Enter a new name for the selected preset:", MessageBoxIconType::NoIcon);

            win.addTextEditor("presetName", "");

            win.addButton("Cancel", 0);
            win.addButton("Ok", 1, KeyPress(KeyPress::returnKey));

            if (win.runModalLoop()) {
                File newFile = selected->getFile().getParentDirectory();

                newFile = newFile.getChildFile(win.getTextEditorContents("presetName")).withFileExtension("fxp");

                selected->getFile().moveFileTo(newFile);

                selected->getParentItem()->itemOpennessChanged(true);
                presetList->repaint();
            }
        }
    }
}
