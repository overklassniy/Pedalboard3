// MappingsDialog.cpp - Dialog window showing all mappings for a plugin.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
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

#include "MappingEntryMidi.h"
#include "MappingEntryOsc.h"
#include "MidiMappingManager.h"
#include "OscMappingManager.h"
#include "Mapping.h"
#include "PluginField.h"
#include "ColourScheme.h"
#include "BypassableInstance.h"
#include "MappingsDialog.h"

//==============================================================================
MappingsDialog::MappingsDialog(MidiMappingManager* manager, OscMappingManager* manager2,
                               AudioProcessorGraph::Node::Ptr  plugin, Array<Mapping*> maps,
                               PluginField* field)
    : midiManager(manager)
    , oscManager(manager2)
    , pluginNode(plugin)
    , mappings(maps)
    , pluginField(field)
    , midiLearnIndex(-1)
{
    mappingsList = std::make_unique<ListBox>(String(), this);
    addAndMakeVisible(mappingsList.get());
    mappingsList->setName("mappingsList");

    addMidiButton = std::make_unique<TextButton>("addMidiButton");
    addAndMakeVisible(addMidiButton.get());
    addMidiButton->setButtonText("add MIDI");
    addMidiButton->addListener(this);

    addOscButton = std::make_unique<TextButton>("addOscButton");
    addAndMakeVisible(addOscButton.get());
    addOscButton->setButtonText("add OSC");
    addOscButton->addListener(this);

    deleteButton = std::make_unique<TextButton>("deleteButton");
    addAndMakeVisible(deleteButton.get());
    deleteButton->setButtonText("remove");
    deleteButton->addListener(this);

    overrideMidiButton = std::make_unique<ToggleButton>("overrideMidiButton");
    addAndMakeVisible(overrideMidiButton.get());
    overrideMidiButton->setButtonText("Override plugin's default MIDI behaviour");
    overrideMidiButton->addListener(this);

    oscMidiAddressLabel = std::make_unique<Label>("oscMidiAddressLabel",
                                                   "OSC MIDI Address:");
    addAndMakeVisible(oscMidiAddressLabel.get());
    oscMidiAddressLabel->setFont(Font(juce::FontOptions().withHeight(15.0f)));
    oscMidiAddressLabel->setJustificationType(Justification::centredLeft);
    oscMidiAddressLabel->setEditable(false, false, false);
    oscMidiAddressLabel->setColour(TextEditor::textColourId, Colours::black);
    oscMidiAddressLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    oscMidiAddress = std::make_unique<TextEditor>("oscMidiAddress");
    addAndMakeVisible(oscMidiAddress.get());
    oscMidiAddress->setMultiLine(false);
    oscMidiAddress->setReturnKeyStartsNewLine(false);
    oscMidiAddress->setReadOnly(false);
    oscMidiAddress->setScrollbarsShown(true);
    oscMidiAddress->setCaretVisible(true);
    oscMidiAddress->setPopupMenuEnabled(true);
    oscMidiAddress->setText({});

    oscHintLabel = std::make_unique<Label>("oscHintLabel",
                                            "(leave blank if you don't want to receive MIDI over OSC)");
    addAndMakeVisible(oscHintLabel.get());
    oscHintLabel->setFont(Font(juce::FontOptions().withHeight(15.0f)));
    oscHintLabel->setJustificationType(Justification::centredRight);
    oscHintLabel->setEditable(false, false, false);
    oscHintLabel->setColour(Label::textColourId, Colours::black);
    oscHintLabel->setColour(TextEditor::textColourId, Colours::black);
    oscHintLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    midiChannelLabel = std::make_unique<Label>("midiChannelLabel",
                                                "MIDI Channel:");
    addAndMakeVisible(midiChannelLabel.get());
    midiChannelLabel->setFont(Font(juce::FontOptions().withHeight(15.0f)));
    midiChannelLabel->setJustificationType(Justification::centredLeft);
    midiChannelLabel->setEditable(false, false, false);
    midiChannelLabel->setColour(TextEditor::textColourId, Colours::black);
    midiChannelLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    midiChannelComboBox = std::make_unique<ComboBox>("midiChannelComboBox");
    addAndMakeVisible(midiChannelComboBox.get());
    midiChannelComboBox->setEditableText(false);
    midiChannelComboBox->setJustificationType(Justification::centredLeft);
    midiChannelComboBox->setTextWhenNothingSelected("Omni");
    midiChannelComboBox->setTextWhenNoChoicesAvailable("(no choices)");
    midiChannelComboBox->addItem("Omni", 1);
    midiChannelComboBox->addItem("1", 2);
    midiChannelComboBox->addItem("2", 3);
    midiChannelComboBox->addItem("3", 4);
    midiChannelComboBox->addItem("4", 5);
    midiChannelComboBox->addItem("5", 6);
    midiChannelComboBox->addItem("6", 7);
    midiChannelComboBox->addItem("7", 8);
    midiChannelComboBox->addItem("8", 9);
    midiChannelComboBox->addItem("9", 10);
    midiChannelComboBox->addItem("10", 11);
    midiChannelComboBox->addItem("11", 12);
    midiChannelComboBox->addItem("12", 13);
    midiChannelComboBox->addItem("13", 14);
    midiChannelComboBox->addItem("14", 15);
    midiChannelComboBox->addItem("15", 16);
    midiChannelComboBox->addItem("16", 17);
    midiChannelComboBox->addListener(this);

    mappingsList->updateContent();
    mappingsList->setOutlineThickness(1);
    mappingsList->setColour(ListBox::outlineColourId, Colour(0x60000000));
    mappingsList->setRowHeight(40);
    mappingsList->setMultipleSelectionEnabled(true);
    mappingsList->setClickingTogglesRowSelection(true);
    mappingsList->setMouseClickGrabsKeyboardFocus(true);
    mappingsList->setWantsKeyboardFocus(true);
    mappingsList->setColour(ListBox::backgroundColourId,
                            ColourScheme::getInstance().colours["Dialog Inner Background"]);

    overrideMidiButton->setToggleState(!pluginField->getMidiEnabledForNode(pluginNode), false);

    {
        auto* proc = dynamic_cast<BypassableInstance*>(pluginNode->getProcessor());

        if (proc)
        {
            oscMidiAddress->setText(oscManager->getMIDIProcessorAddress(proc));

            midiChannelComboBox->setSelectedId(proc->getMIDIChannel() + 1);
        }
    }
    oscMidiAddress->addListener(this);

    setSize(738, 400);
}

MappingsDialog::~MappingsDialog()
{
    if (midiLearnIndex > -1)
        midiManager->unregisterMidiLearnCallback(this);

    mappingsList.reset();
    addMidiButton.reset();
    addOscButton.reset();
    deleteButton.reset();
    overrideMidiButton.reset();
    oscMidiAddressLabel.reset();
    oscMidiAddress.reset();
    oscHintLabel.reset();
    midiChannelLabel.reset();
    midiChannelComboBox.reset();
}

//==============================================================================
void MappingsDialog::paint(Graphics& g)
{
    g.fillAll(Colour(0xffeeece1));

    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

void MappingsDialog::resized()
{
    mappingsList->setBounds(8, 40, getWidth() - 16, getHeight() - 79);
    addMidiButton->setBounds(8, getHeight() - 31, 80, 24);
    addOscButton->setBounds(96, getHeight() - 31, 72, 24);
    deleteButton->setBounds(getWidth() - 70, getHeight() - 31, 62, 24);
    overrideMidiButton->setBounds(317 - ((282) / 2), getHeight() - 31, 282, 24);
    oscMidiAddressLabel->setBounds(8, 8, 136, 24);
    oscMidiAddress->setBounds(136, 8, getWidth() - 498, 24);
    oscHintLabel->setBounds(getWidth() - 362, 8, 360, 24);
    midiChannelLabel->setBounds(472, getHeight() - 31, 104, 24);
    midiChannelComboBox->setBounds(576, getHeight() - 31, 64, 24);
}

void MappingsDialog::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == addMidiButton.get())
    {
        auto* mapping = new MidiMapping(midiManager,
                                        pluginField->getFilterGraph(),
                                        pluginNode->nodeID.uid,
                                        0,
                                        0,
                                        false,
                                        midiChannelComboBox->getSelectedId() - 1,
                                        0.0f,
                                        1.0f);
        midiManager->registerMapping(0, mapping);
        mappings.add(mapping);
        pluginField->addMapping(mapping);
        mappingsList->updateContent();
        repaint();
    }
    else if (buttonThatWasClicked == addOscButton.get())
    {
        auto* mapping = new OscMapping(oscManager,
                                       pluginField->getFilterGraph(),
                                       pluginNode->nodeID.uid,
                                       0,
                                       "",
                                       0);
        oscManager->registerMapping("", mapping);
        mappings.add(mapping);
        pluginField->addMapping(mapping);
        mappingsList->updateContent();
        repaint();
    }
    else if (buttonThatWasClicked == deleteButton.get())
    {
        int i;
        Array<int> mappingsToDelete;
        const SparseSet<int> selectedRows = mappingsList->getSelectedRows();

        for (i = 0; i < selectedRows.size(); ++i)
        {
            jassert(selectedRows[i] < mappings.size());

            mappingsToDelete.add(selectedRows[i]);
        }

        mappingsToDelete.sort(comparator);
        for (i = (mappingsToDelete.size() - 1); i >= 0; --i)
        {
            pluginField->removeMapping(mappings[mappingsToDelete[i]]);
            mappings.remove(mappingsToDelete[i]);
        }

        mappingsList->updateContent();
    }
    else if (buttonThatWasClicked == overrideMidiButton.get())
    {
        pluginField->enableMidiForNode(pluginNode,
                                       overrideMidiButton->getToggleState());
    }
}

void MappingsDialog::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == midiChannelComboBox.get())
    {
        int i;
        int channel = midiChannelComboBox->getSelectedId() - 1;
        auto* proc = dynamic_cast<BypassableInstance*>(pluginNode->getProcessor());

        if (proc)
            proc->setMIDIChannel(channel);

        for (i = 0; i < mappings.size(); ++i)
        {
            auto* tempMapping = dynamic_cast<MidiMapping*>(mappings[i]);

            if (tempMapping)
                tempMapping->setChannel(channel);
        }
    }
}

//==============================================================================
int MappingsDialog::getNumRows()
{
    return mappings.size();
}

//------------------------------------------------------------------------------
void MappingsDialog::paintListBoxItem(int rowNumber,
                                      Graphics& g,
                                      int width,
                                      int height,
                                      bool rowIsSelected)
{
    Colour highlight = ColourScheme::getInstance().colours["List Selected Colour"];

    if (rowIsSelected)
    {
        ColourGradient basil(highlight.brighter(0.4f),
                             0.0f,
                             0.0f,
                             highlight.darker(0.125f),
                             0.0f,
                             static_cast<float>(height),
                             false);

        g.setGradientFill(basil);

        g.fillAll();
    }
    else if (rowNumber % 2)
        g.fillAll(Colour(0x10000000));
}

//------------------------------------------------------------------------------
Component* MappingsDialog::refreshComponentForRow(int rowNumber,
                                                  bool isRowSelected,
                                                  Component* existingComponentToUpdate)
{
    int i;
    int numParams;
    int selectedParam;
    Component* retval = nullptr;
    auto* midiMapping = dynamic_cast<MidiMapping*>(mappings[rowNumber]);
    auto* oscMapping = dynamic_cast<OscMapping*>(mappings[rowNumber]);

    if (existingComponentToUpdate)
        delete existingComponentToUpdate;

    if (midiMapping)
    {
        retval = new MappingEntryMidi(this,
                                      rowNumber,
                                      midiMapping->getCc(),
                                      midiMapping->getLatched(),
                                      midiMapping->getLowerBound(),
                                      midiMapping->getUpperBound());

        // Fill out the parameters combo box.
        auto* proc = pluginNode->getProcessor();
        numParams = static_cast<int>(proc->getParameters().size());
        for (i = 0; i < numParams; ++i)
            static_cast<MappingEntryMidi*>(retval)->addParameter(proc->getParameters()[i]->getName(128));
        static_cast<MappingEntryMidi*>(retval)->addParameter("Bypass");

        selectedParam = midiMapping->getParameter();
        if (selectedParam == -1)
            selectedParam = numParams;
        static_cast<MappingEntryMidi*>(retval)->selectParameter(selectedParam);
    }
    else if (oscMapping)
    {
        retval = new MappingEntryOsc(this,
                                     rowNumber,
                                     oscMapping->getAddress(),
                                     oscMapping->getParameterIndex(),
                                     oscManager->getReceivedAddresses());

        // Fill out the parameters combo box.
        auto* proc = pluginNode->getProcessor();
        numParams = static_cast<int>(proc->getParameters().size());
        for (i = 0; i < numParams; ++i)
            static_cast<MappingEntryOsc*>(retval)->addParameter(proc->getParameters()[i]->getName(128));
        static_cast<MappingEntryOsc*>(retval)->addParameter("Bypass");

        selectedParam = oscMapping->getParameter();
        if (selectedParam == -1)
            selectedParam = numParams;
        static_cast<MappingEntryOsc*>(retval)->selectParameter(selectedParam);
    }

    return retval;
}

//------------------------------------------------------------------------------
void MappingsDialog::listBoxItemClicked(int row, const MouseEvent& e)
{
    mappingsList->selectRow(row, false, !e.mods.isCtrlDown());
}

//------------------------------------------------------------------------------
void MappingsDialog::backgroundClicked(const MouseEvent&)
{
    mappingsList->deselectAllRows();
}

//------------------------------------------------------------------------------
void MappingsDialog::textEditorTextChanged(TextEditor& editor)
{
    auto* proc = dynamic_cast<BypassableInstance*>(pluginNode->getProcessor());

    if (proc)
        oscManager->registerMIDIProcessor(editor.getText(), proc);
}

//------------------------------------------------------------------------------
void MappingsDialog::setParameter(int index, int val)
{
    auto* proc = pluginNode->getProcessor();
    if (val == static_cast<int>(proc->getParameters().size()))
        val = -1;
    mappings[index]->setParameter(val);
}

//------------------------------------------------------------------------------
void MappingsDialog::setCc(int index, int val)
{
    auto* midiMapping = dynamic_cast<MidiMapping*>(mappings[index]);

    if (midiMapping)
        midiMapping->setCc(val);
    else
        jassertfalse;
}

//------------------------------------------------------------------------------
void MappingsDialog::setLatch(int index, bool val)
{
    auto* midiMapping = dynamic_cast<MidiMapping*>(mappings[index]);

    if (midiMapping)
        midiMapping->setLatched(val);
    else
        jassertfalse;
}

//------------------------------------------------------------------------------
void MappingsDialog::setLowerBound(int index, float val)
{
    auto* midiMapping = dynamic_cast<MidiMapping*>(mappings[index]);

    if (midiMapping)
        midiMapping->setLowerBound(val);
    else
        jassertfalse;
}

//------------------------------------------------------------------------------
void MappingsDialog::setUpperBound(int index, float val)
{
    auto* midiMapping = dynamic_cast<MidiMapping*>(mappings[index]);

    if (midiMapping)
        midiMapping->setUpperBound(val);
    else
        jassertfalse;
}

//------------------------------------------------------------------------------
void MappingsDialog::setAddress(int index, const String& address)
{
    auto* oscMapping = dynamic_cast<OscMapping*>(mappings[index]);

    if (oscMapping)
        oscMapping->setAddress(address);
    else
        jassertfalse;
}

//------------------------------------------------------------------------------
void MappingsDialog::setOscParameter(int index, int val)
{
    auto* oscMapping = dynamic_cast<OscMapping*>(mappings[index]);

    if (oscMapping)
        oscMapping->setParameterIndex(val);
    else
        jassertfalse;
}

//------------------------------------------------------------------------------
void MappingsDialog::activateMidiLearn(int index)
{
    if (index > -1)
    {
        midiLearnIndex = index;

        midiManager->registerMidiLearnCallback(this);
    }
}

//------------------------------------------------------------------------------
void MappingsDialog::deactivateMidiLearn(int /*index*/)
{
    midiLearnIndex = -1;

    midiManager->unregisterMidiLearnCallback(this);
}

//------------------------------------------------------------------------------
void MappingsDialog::midiCcReceived(int val)
{
    if (midiLearnIndex > -1)
    {
        auto* mapping = dynamic_cast<MidiMapping*>(mappings[midiLearnIndex]);

        if (mapping)
        {
            mapping->setCc(val);
            triggerAsyncUpdate();
        }

        midiLearnIndex = -1;
    }
}

//------------------------------------------------------------------------------
void MappingsDialog::handleAsyncUpdate()
{
    updateListBox();
}

//------------------------------------------------------------------------------
void MappingsDialog::updateListBox()
{
    mappingsList->updateContent();
}

//------------------------------------------------------------------------------
int MappingsDialog::MappingComparator::compareElements(int first, int second)
{
    int retval = 0;

    if (first < second)
        retval = -1;
    else if (first > second)
        retval = 1;

    return retval;
}
