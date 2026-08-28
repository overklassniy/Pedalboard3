// BlacklistWindow.h - Window for managing the plugin blacklist.
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

#include <JuceHeader.h>

/// ListBox model for displaying blacklisted plugins.
class BlacklistListModel : public ListBoxModel {
  public:
    BlacklistListModel() { refresh(); }

    /// Reloads the blacklist from PluginBlacklist and updates the list.
    void refresh();

    int getNumRows() override { return items.size(); }

    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;

    void listBoxItemClicked(int row, const MouseEvent& e) override {}

    String getItemAt(int index) const { return index >= 0 && index < items.size() ? items[index] : String(); }

  private:
    StringArray items;
};

/// Component for managing the plugin blacklist.
class BlacklistComponent : public Component, public Button::Listener {
  public:
    BlacklistComponent();
    ~BlacklistComponent() override = default;

    void paint(Graphics& g) override;
    void resized() override;
    void buttonClicked(Button* button) override;

  private:
    void refreshList();

    std::unique_ptr<Label> titleLabel;
    std::unique_ptr<Label> infoLabel;
    std::unique_ptr<ListBox> listBox;
    std::unique_ptr<TextButton> removeButton;
    std::unique_ptr<TextButton> clearAllButton;
    std::unique_ptr<TextButton> closeButton;

    BlacklistListModel listModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlacklistComponent)
};

/// Window for managing plugin blacklist.
class BlacklistWindow : public DocumentWindow {
  public:
    BlacklistWindow();
    ~BlacklistWindow() override = default;

    void closeButtonPressed() override { setVisible(false); }

    static void showWindow();

  private:
    static std::unique_ptr<BlacklistWindow> instance;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlacklistWindow)
};
