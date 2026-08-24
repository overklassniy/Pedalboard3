// MappingEntryOsc.cpp - OSC mapping entry UI component.
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

#include "MappingEntryOsc.h"

#include "ColourScheme.h"
#include "MappingsDialog.h"

MappingEntryOsc::MappingEntryOsc(MappingsDialog* dlg, int arrayIndex, const String& oscAddress, int oscParam,
                                 const StringArray& possibleAddresses)
    : mappingsDialog(dlg), index(arrayIndex) {
    paramComboBox = std::make_unique<ComboBox>("paramComboBox");
    addAndMakeVisible(paramComboBox.get());
    paramComboBox->setEditableText(false);
    paramComboBox->setJustificationType(Justification::centredLeft);
    paramComboBox->setTextWhenNothingSelected({});
    paramComboBox->setTextWhenNoChoicesAvailable("(no choices)");
    paramComboBox->addListener(this);

    addressLabel = std::make_unique<Label>("addressLabel", "OSC Address:");
    addAndMakeVisible(addressLabel.get());
    addressLabel->setFont(Font(juce::FontOptions().withHeight(15.0f)));
    addressLabel->setJustificationType(Justification::centredLeft);
    addressLabel->setEditable(false, false, false);
    addressLabel->setColour(TextEditor::textColourId, Colours::black);
    addressLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    addressEditor = std::make_unique<ComboBox>("addressEditor");
    addAndMakeVisible(addressEditor.get());
    addressEditor->setEditableText(true);
    addressEditor->setJustificationType(Justification::centredLeft);
    addressEditor->setTextWhenNothingSelected({});
    addressEditor->setTextWhenNoChoicesAvailable("(no choices)");
    addressEditor->addListener(this);

    oscParamLabel = std::make_unique<Label>("oscParamLabel", "Parameter:");
    addAndMakeVisible(oscParamLabel.get());
    oscParamLabel->setFont(Font(juce::FontOptions().withHeight(15.0f)));
    oscParamLabel->setJustificationType(Justification::centredLeft);
    oscParamLabel->setEditable(false, false, false);
    oscParamLabel->setColour(TextEditor::textColourId, Colours::black);
    oscParamLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    oscParamSlider = std::make_unique<Slider>("oscParamSlider");
    addAndMakeVisible(oscParamSlider.get());
    oscParamSlider->setRange(0, 100, 1);
    oscParamSlider->setSliderStyle(Slider::IncDecButtons);
    oscParamSlider->setTextBoxStyle(Slider::TextBoxLeft, false, 30, 20);
    oscParamSlider->addListener(this);

    int i;
    bool textExists = false;

    setInterceptsMouseClicks(false, true);

    for (i = 0; i < possibleAddresses.size(); ++i)
        addressEditor->addItem(possibleAddresses[i], i + 1);

    if (oscAddress != "") {
        for (i = 0; i < addressEditor->getNumItems(); ++i) {
            if (addressEditor->getItemText(i) == oscAddress) {
                addressEditor->setSelectedId(i + 1);
                textExists = true;
                break;
            }
        }
        if (!textExists) {
            addressEditor->addItem(oscAddress, addressEditor->getNumItems() + 1);
            addressEditor->setSelectedId(addressEditor->getNumItems());
        }
    }

    addressEditor->addListener(this);

    addressLabel->setColour(TextEditor::textColourId, ColourScheme::getInstance().colours["Text Colour"]);
    oscParamLabel->setColour(TextEditor::textColourId, ColourScheme::getInstance().colours["Text Colour"]);
    oscParamSlider->setColour(Slider::textBoxTextColourId, ColourScheme::getInstance().colours["Text Colour"]);
    oscParamSlider->setColour(Slider::textBoxBackgroundColourId,
                              ColourScheme::getInstance().colours["Text Editor Colour"]);

    oscParamSlider->setValue(static_cast<double>(oscParam), dontSendNotification);

    setSize(728, 400);
}

MappingEntryOsc::~MappingEntryOsc() {
    paramComboBox.reset();
    addressLabel.reset();
    addressEditor.reset();
    oscParamLabel.reset();
    oscParamSlider.reset();
}

void MappingEntryOsc::paint(Graphics& g) {
    g.setColour(ColourScheme::getInstance().colours["Vector Colour"]);

    g.setColour(Colour(0x80000000));
    g.strokePath(internalPath1, PathStrokeType(5.0000f, PathStrokeType::curved, PathStrokeType::rounded));
}

void MappingEntryOsc::resized() {
    paramComboBox->setBounds(576, 8, 144, 24);
    addressLabel->setBounds(8, 8, 96, 24);
    addressEditor->setBounds(104, 8, 272, 24);
    oscParamLabel->setBounds(376, 8, 80, 24);
    oscParamSlider->setBounds(456, 8, 80, 24);
    internalPath1.clear();
    internalPath1.startNewSubPath(556.0f, 12.0f);
    internalPath1.lineTo(562.0f, 20.0f);
    internalPath1.lineTo(556.0f, 28.0f);
}

void MappingEntryOsc::comboBoxChanged(ComboBox* comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == paramComboBox.get()) {
        mappingsDialog->setParameter(index, paramComboBox->getSelectedId() - 1);
    } else if (comboBoxThatHasChanged == addressEditor.get()) {
        int i;
        bool textExists = false;
        String tempstr = addressEditor->getText();

        if (tempstr != "") {
            // Add the new text to the combobox if the user typed it in.
            for (i = 0; i < addressEditor->getNumItems(); ++i) {
                if (addressEditor->getItemText(i) == tempstr) {
                    textExists = true;
                    break;
                }
            }
            if (!textExists)
                addressEditor->addItem(tempstr, addressEditor->getNumItems() + 1);

            // Update the OscManager.
            mappingsDialog->setAddress(index, addressEditor->getText());
        }
    }
}

void MappingEntryOsc::sliderValueChanged(Slider* sliderThatWasMoved) {
    if (sliderThatWasMoved == oscParamSlider.get()) {
        mappingsDialog->setOscParameter(index, static_cast<int>(sliderThatWasMoved->getValue()));
    }
}

void MappingEntryOsc::textEditorTextChanged(TextEditor& editor) {
    mappingsDialog->setAddress(index, editor.getText());
}

void MappingEntryOsc::textEditorReturnKeyPressed(TextEditor& editor) {
    mappingsDialog->setAddress(index, editor.getText());
}

void MappingEntryOsc::textEditorEscapeKeyPressed(TextEditor& /*editor*/) {
    mappingsDialog->updateListBox();
}

void MappingEntryOsc::textEditorFocusLost(TextEditor& editor) {
    mappingsDialog->setAddress(index, editor.getText());
}

void MappingEntryOsc::addParameter(const String& param) {
    String paramName = param;

    if (paramName.isEmpty())
        paramName = "<no name>";

    paramComboBox->addItem(paramName, paramComboBox->getNumItems() + 1);
}

void MappingEntryOsc::selectParameter(int index) {
    paramComboBox->setSelectedId(index + 1, true);
}
