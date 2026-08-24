// UserPresetWindow.h - User preset window UI.
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

#ifndef USERPRESETWINDOW_H_
#define USERPRESETWINDOW_H_

#include "ColourScheme.h"

#include <JuceHeader.h>

/// Component for managing user-saved plugin presets.
///
/// Displays a tree view of plugins and their presets, with buttons to
/// copy, remove, import, export, and rename presets.
class UserPresetWindow : public Component, public juce::Button::Listener {
  public:
    /// Creates the preset window using the given known plugin list.
    ///
    /// @param knownPlugins The known plugin list used to populate the import plugin selector.
    UserPresetWindow(KnownPluginList* knownPlugins);
    /// Destructor. Detaches the tree root from the TreeView.
    ~UserPresetWindow() override;

    /// Paints the window background.
    ///
    /// @param g The graphics context to draw with.
    void paint(Graphics& g) override;
    /// Lays out the tree view and buttons.
    void resized() override;
    /// Handles copy, remove, import, export, and rename button clicks.
    ///
    /// @param buttonThatWasClicked The button that was clicked.
    void buttonClicked(Button* buttonThatWasClicked) override;

  private:
    /// The preset item for the TreeView.
    class PresetItem : public TreeViewItem {
      public:
        PresetItem(const File& preset) : name(preset.getFileNameWithoutExtension()), presetFile(preset) {}

        ~PresetItem() override = default;

        /// Returns false because preset items are leaf nodes.
        bool mightContainSubItems() override { return false; }

        /// Draws the preset name with a selection gradient when selected.
        ///
        /// @param g The graphics context to draw with.
        /// @param width The width of the item in pixels.
        /// @param height The height of the item in pixels.
        void paintItem(Graphics& g, int width, int height) override {
            if (isSelected()) {
                Colour highlight = ColourScheme::getInstance().colours["List Selected Colour"];
                ColourGradient basil(highlight.brighter(0.4f), 0.0f, 0.0f, highlight.darker(0.125f), 0.0f,
                                     static_cast<float>(height), false);

                g.setGradientFill(basil);

                g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(width) - 4.0f, static_cast<float>(height), 4.0f);
            }

            g.setColour(ColourScheme::getInstance().colours["Text Colour"]);
            g.setFont(juce::FontOptions().withHeight(16.0f));
            g.drawText(name, 4, 0, width, height, Justification::centredLeft, false);
        }

        /// Returns this preset's file.
        File& getFile() { return presetFile; }

      private:
        /// The name of this preset.
        String name;

        /// This preset's file.
        File presetFile;
    };

    /// The plugin item for the TreeView.
    class PluginItem : public TreeViewItem {
      public:
        PluginItem(const File& plugin) : name(plugin.getFileName()), pluginDir(plugin) {
            setLinesDrawnForSubItems(true);
            setOpen(true);
        }

        ~PluginItem() override = default;

        /// Returns true because plugin items contain preset sub-items.
        bool mightContainSubItems() override { return true; }

        /// Returns false because plugin items act as group headers, not selections.
        bool canBeSelected() const override { return false; }

        /// Rebuilds sub-items by scanning for .fxp files in this plugin's directory.
        ///
        /// @param isNowOpen Whether the item is now open.
        void itemOpennessChanged(bool isNowOpen) override {
            Array<File> presets;

            clearSubItems();

            pluginDir.findChildFiles(presets, File::findFiles, false, "*.fxp");
            for (const auto& preset : presets)
                addSubItem(new PresetItem(preset));
        }

        /// Draws the plugin name in bold text.
        ///
        /// @param g The graphics context to draw with.
        /// @param width The width of the item in pixels.
        /// @param height The height of the item in pixels.
        void paintItem(Graphics& g, int width, int height) override {
            g.setColour(ColourScheme::getInstance().colours["Text Colour"]);
            g.setFont(juce::FontOptions().withHeight(16.0f).withStyle("Bold"));
            g.drawText(name, 0, 0, width, height, Justification::centredLeft, false);
        }

      private:
        /// The name of this plugin.
        String name;

        /// This plugin's preset directory.
        File pluginDir;
    };

    /// The root item in the presetList.
    class RootItem : public TreeViewItem {
      public:
        RootItem() {
            setLinesDrawnForSubItems(false);
            setOpen(true);
        }

        ~RootItem() override = default;

        /// Returns true because the root contains plugin sub-items.
        bool mightContainSubItems() override { return true; }

        /// Rebuilds sub-items by scanning for plugin directories under the presets folder.
        ///
        /// @param isNowOpen Whether the item is now open.
        void itemOpennessChanged(bool isNowOpen) override {
            File presetDir = File::getSpecialLocation(File::userApplicationDataDirectory)
                                 .getChildFile("Pedalboard3")
                                 .getChildFile("presets");
            Array<File> pluginDirs;

            clearSubItems();

            presetDir.findChildFiles(pluginDirs, File::findDirectories, false);
            for (const auto& dir : pluginDirs)
                addSubItem(new PluginItem(dir));
        }
    };

    /// The root TreeViewItem.
    RootItem treeRoot;

    /// Plugin list used to populate the import plugin selector.
    KnownPluginList* knownPluginList;

    std::unique_ptr<TreeView> presetList;
    std::unique_ptr<TextButton> copyButton;
    std::unique_ptr<TextButton> removeButton;
    std::unique_ptr<TextButton> importButton;
    std::unique_ptr<TextButton> exportButton;
    std::unique_ptr<TextButton> renameButton;

    JUCE_LEAK_DETECTOR(UserPresetWindow)
};

#endif
