// MappingsDialog.h - Dialog window showing all mappings for a plugin.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
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

#ifndef MAPPINGSDIALOG_H_
#define MAPPINGSDIALOG_H_

#include "MidiMappingManager.h"

#include <JuceHeader.h>

class OscMappingManager;
class PluginField;
class Mapping;

/// Dialog showing all MIDI and OSC mappings for a single plugin.
class MappingsDialog : public Component,
                       public ListBoxModel,
                       public TextEditor::Listener,
                       public MidiMappingManager::MidiLearnCallback,
                       public AsyncUpdater,
                       public juce::Button::Listener,
                       public juce::ComboBox::Listener {
  public:
    /// Creates the mappings dialog for a single plugin.
    ///
    /// @param manager The MidiMappingManager for any MIDI mappings.
    /// @param manager2 The OscMappingManager for any OSC mappings.
    /// @param plugin The graph node whose mappings are displayed.
    /// @param maps The array of existing mappings.
    /// @param field The PluginField to apply changes to.
    MappingsDialog(MidiMappingManager* manager, OscMappingManager* manager2, AudioProcessorGraph::Node::Ptr plugin,
                   Array<Mapping*> maps, PluginField* field);
    ~MappingsDialog() override;

    /// Returns the number of active mappings.
    int getNumRows() override;
    /// Draws the selection highlight and alternating row background.
    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
    /// Creates the MIDI or OSC entry component for a single row.
    Component* refreshComponentForRow(int rowNumber, bool isRowSelected, Component* existingComponentToUpdate) override;
    /// Selects the clicked row, supporting Ctrl multi-selection.
    void listBoxItemClicked(int row, const MouseEvent& e) override;
    /// Deselects all rows when the background is clicked.
    void backgroundClicked(const MouseEvent& e) override;

    /// Registers the typed OSC address with the OscMappingManager so it can
    /// receive MIDI over OSC.
    void textEditorTextChanged(TextEditor& editor) override;

    /// Sets the indexed mapping's parameter.
    ///
    /// @param index The index of the mapping to update.
    /// @param val The parameter index to set (-1 for bypass).
    void setParameter(int index, int val);
    /// Sets the indexed mapping's CC.
    ///
    /// @param index The index of the mapping to update.
    /// @param val The MIDI CC number to set.
    void setCc(int index, int val);
    /// Sets the indexed mapping's latch value.
    ///
    /// @param index The index of the mapping to update.
    /// @param val Whether the CC should be latched.
    void setLatch(int index, bool val);
    /// Sets the indexed mapping's lower bound.
    ///
    /// @param index The index of the mapping to update.
    /// @param val The lower bound value to set.
    void setLowerBound(int index, float val);
    /// Sets the indexed mapping's upper bound.
    ///
    /// @param index The index of the mapping to update.
    /// @param val The upper bound value to set.
    void setUpperBound(int index, float val);
    /// Sets the indexed mapping's OSC address.
    ///
    /// @param index The index of the mapping to update.
    /// @param address The OSC address to set.
    void setAddress(int index, const String& address);
    /// Sets the indexed mapping's OSC parameter index.
    ///
    /// @param index The index of the mapping to update.
    /// @param val The OSC parameter index to set.
    void setOscParameter(int index, int val);

    /// Activates MIDI learn for the indexed mapping.
    ///
    /// @param index The index of the mapping to learn a CC for.
    void activateMidiLearn(int index);
    /// Deactivates MIDI learn for the indexed mapping.
    ///
    /// @param index The index of the mapping to stop learning a CC for.
    void deactivateMidiLearn(int index);
    /// The method which gets called when a MIDI learn CC is received.
    void midiCcReceived(int val) override;
    /// Used to refresh the mappings list when a MIDI learn CC is received.
    void handleAsyncUpdate() override;

    /// Updates the listbox contents.
    void updateListBox();

    void paint(Graphics& g) override;
    void resized() override;
    /// Adds or removes mappings, or toggles MIDI override, depending on which
    /// button was clicked.
    void buttonClicked(Button* buttonThatWasClicked) override;
    /// Updates the MIDI channel for the plugin and all its MIDI mappings.
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

  private:
    /// Comparator used to sort selected row indices before deletion.
    struct MappingComparator {
        /// Returns -1, 0, or 1 depending on whether first is less than,
        /// equal to, or greater than second.
        ///
        /// @param first The first row index to compare.
        /// @param second The second row index to compare.
        /// @return -1 if first is less than second, 0 if equal, 1 if greater.
        int compareElements(int first, int second);
    };

    /// Instance used to sort selected rows in descending order so deletion
    /// does not shift remaining indices.
    MappingComparator comparator;

    /// The MidiMappingsManager for any MidiMappings.
    MidiMappingManager* midiManager;
    /// The OscMappingsManager for any OscMappings.
    OscMappingManager* oscManager;
    /// The plugin this dialog refers to.
    AudioProcessorGraph::Node::Ptr pluginNode;
    /// The mappings this dialog displays.
    Array<Mapping*> mappings;
    /// The PluginField to apply any mapping changes to.
    PluginField* pluginField;

    /// If we are doing a midi learn, this holds the index of the associated
    /// mapping. Otherwise it will be -1.
    int midiLearnIndex;

    std::unique_ptr<ListBox> mappingsList;
    std::unique_ptr<TextButton> addMidiButton;
    std::unique_ptr<TextButton> addOscButton;
    std::unique_ptr<TextButton> deleteButton;
    std::unique_ptr<ToggleButton> overrideMidiButton;
    std::unique_ptr<Label> oscMidiAddressLabel;
    std::unique_ptr<TextEditor> oscMidiAddress;
    std::unique_ptr<Label> oscHintLabel;
    std::unique_ptr<Label> midiChannelLabel;
    std::unique_ptr<ComboBox> midiChannelComboBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappingsDialog)
};

#endif
