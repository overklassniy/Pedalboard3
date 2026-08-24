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
                       public juce::ComboBox::Listener
{
public:
    MappingsDialog(MidiMappingManager* manager, OscMappingManager* manager2,
                   AudioProcessorGraph::Node::Ptr  plugin, Array<Mapping*> maps,
                   PluginField* field);
    ~MappingsDialog() override;

    /// Returns the number of active mappings.
    int getNumRows() override;
    /// Draws a row.
    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height,
                          bool rowIsSelected) override;
    /// Returns the component for a single row.
    Component* refreshComponentForRow(int rowNumber, bool isRowSelected,
                                      Component* existingComponentToUpdate) override;
    /// So the user can select rows.
    void listBoxItemClicked(int row, const MouseEvent& e) override;
    /// So the user can deselect rows.
    void backgroundClicked(const MouseEvent& e) override;

    /// So the OscMappingManager can get updated with the correct MIDI over
    /// OSC address.
    void textEditorTextChanged(TextEditor& editor) override;

    /// Sets the indexed mapping's parameter.
    void setParameter(int index, int val);
    /// Sets the indexed mapping's CC.
    void setCc(int index, int val);
    /// Sets the indexed mapping's latch value.
    void setLatch(int index, bool val);
    /// Sets the indexed mapping's lower bound.
    void setLowerBound(int index, float val);
    /// Sets the indexed mapping's upper bound.
    void setUpperBound(int index, float val);
    /// Sets the indexed mapping's OSC address.
    void setAddress(int index, const String& address);
    /// Sets the indexed mapping's OSC parameter index.
    void setOscParameter(int index, int val);

    /// Activates MIDI learn for the indexed mapping.
    void activateMidiLearn(int index);
    /// Deactivates MIDI learn for the indexed mapping.
    void deactivateMidiLearn(int index);
    /// The method which gets called when a MIDI learn CC is received.
    void midiCcReceived(int val) override;
    /// Used to refresh the mappings list when a MIDI learn CC is received.
    void handleAsyncUpdate() override;

    /// Updates the listbox contents.
    void updateListBox();

    void paint(Graphics& g) override;
    void resized() override;
    void buttonClicked(Button* buttonThatWasClicked) override;
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

private:
    /// For sorting the results of mappingsList->getSelectedRows().
    struct MappingComparator
    {
        /// For sorting the results of mappingsList->getSelectedRows().
        int compareElements(int first, int second);
    };

    /// For sorting the results of mappingsList->getSelectedRows().
    MappingComparator comparator;

    /// The MidiMappingsManager for any MidiMappings.
    MidiMappingManager* midiManager;
    /// The OscMappingsManager for any OscMappings.
    OscMappingManager* oscManager;
    /// The plugin this dialog refers to.
    AudioProcessorGraph::Node::Ptr  pluginNode;
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
