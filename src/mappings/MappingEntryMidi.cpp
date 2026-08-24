// MappingEntryMidi.cpp - MIDI mapping entry UI component.
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

#include "MappingEntryMidi.h"

#include "ColourScheme.h"
#include "MappingsDialog.h"

MappingEntryMidi::MappingEntryMidi(MappingsDialog* dlg, int arrayIndex, int cc, bool latch, float lowerBound,
                                   float upperBound)
    : mappingsDialog(dlg), index(arrayIndex), midiLearn(false) {
    ccComboBox = std::make_unique<ComboBox>("ccComboBox");
    addAndMakeVisible(ccComboBox.get());
    ccComboBox->setEditableText(false);
    ccComboBox->setJustificationType(Justification::centredLeft);
    ccComboBox->setTextWhenNothingSelected({});
    ccComboBox->setTextWhenNoChoicesAvailable("(no choices)");
    ccComboBox->addItem("<< MIDI Learn >>", 1);
    ccComboBox->addItem("0: Bank Select", 2);
    ccComboBox->addItem("1: Mod Wheel", 3);
    ccComboBox->addItem("2: Breath", 4);
    ccComboBox->addItem("3:", 5);
    ccComboBox->addItem("4: Foot Pedal", 6);
    ccComboBox->addItem("5: Portamento", 7);
    ccComboBox->addItem("6: Data Entry", 8);
    ccComboBox->addItem("7: Volume", 9);
    ccComboBox->addItem("8: Balance", 10);
    ccComboBox->addItem("9:", 11);
    ccComboBox->addItem("10: Pan", 12);
    ccComboBox->addItem("11: Expression", 13);
    ccComboBox->addItem("12: Effect Control 1", 14);
    ccComboBox->addItem("13: EffectControl 2", 15);
    ccComboBox->addItem("14:", 16);
    ccComboBox->addItem("15:", 17);
    ccComboBox->addItem("16: General Purpose 1", 18);
    ccComboBox->addItem("17: General Purpose 2", 19);
    ccComboBox->addItem("18: General Purpose 3", 20);
    ccComboBox->addItem("19: General Purpose 4", 21);
    ccComboBox->addItem("20:", 22);
    ccComboBox->addItem("21:", 23);
    ccComboBox->addItem("22:", 24);
    ccComboBox->addItem("23:", 25);
    ccComboBox->addItem("24:", 26);
    ccComboBox->addItem("25:", 27);
    ccComboBox->addItem("26:", 28);
    ccComboBox->addItem("27:", 29);
    ccComboBox->addItem("28:", 30);
    ccComboBox->addItem("29:", 31);
    ccComboBox->addItem("30:", 32);
    ccComboBox->addItem("31:", 33);
    ccComboBox->addItem("32: Bank Select (fine)", 34);
    ccComboBox->addItem("33: Mod Wheel (fine)", 35);
    ccComboBox->addItem("34: Breath (fine)", 36);
    ccComboBox->addItem("35:", 37);
    ccComboBox->addItem("36: Foot Pedal (fine)", 38);
    ccComboBox->addItem("37: Portamento (fine)", 39);
    ccComboBox->addItem("38: Data Entry (fine)", 40);
    ccComboBox->addItem("39: Volume (fine)", 41);
    ccComboBox->addItem("40: Balance (fine)", 42);
    ccComboBox->addItem("41:", 43);
    ccComboBox->addItem("42: Pan (fine)", 44);
    ccComboBox->addItem("43: Expression (fine)", 45);
    ccComboBox->addItem("44: Effect Control 1 (fine)", 46);
    ccComboBox->addItem("45: Effect Control 2 (fine)", 47);
    ccComboBox->addItem("46:", 48);
    ccComboBox->addItem("47:", 49);
    ccComboBox->addItem("48:", 50);
    ccComboBox->addItem("49:", 51);
    ccComboBox->addItem("50:", 52);
    ccComboBox->addItem("51:", 53);
    ccComboBox->addItem("52:", 54);
    ccComboBox->addItem("53:", 55);
    ccComboBox->addItem("54:", 56);
    ccComboBox->addItem("55:", 57);
    ccComboBox->addItem("56:", 58);
    ccComboBox->addItem("57:", 59);
    ccComboBox->addItem("58:", 60);
    ccComboBox->addItem("59:", 61);
    ccComboBox->addItem("60:", 62);
    ccComboBox->addItem("61:", 63);
    ccComboBox->addItem("62:", 64);
    ccComboBox->addItem("63:", 65);
    ccComboBox->addItem("64: Hold Pedal", 66);
    ccComboBox->addItem("65: Portamento (on/off)", 67);
    ccComboBox->addItem("66: Sustenuto Pedal", 68);
    ccComboBox->addItem("67: Soft Pedal", 69);
    ccComboBox->addItem("68: Legato Pedal", 70);
    ccComboBox->addItem("69: Hold 2 Pedal", 71);
    ccComboBox->addItem("70: Sound Variation", 72);
    ccComboBox->addItem("71: Sound Timbre", 73);
    ccComboBox->addItem("72: Sound Release Time", 74);
    ccComboBox->addItem("73: Sound Attack Time", 75);
    ccComboBox->addItem("74: Sound Brightness", 76);
    ccComboBox->addItem("75: Sound Control 6", 77);
    ccComboBox->addItem("76: Sound Control 7", 78);
    ccComboBox->addItem("77: Sound Control 8", 79);
    ccComboBox->addItem("78: Sound Control 9", 80);
    ccComboBox->addItem("79: Sound Control 10", 81);
    ccComboBox->addItem("80: General Purpose Button 1", 82);
    ccComboBox->addItem("81: General Purpose Button 2", 83);
    ccComboBox->addItem("82: General Purpose Button 3", 84);
    ccComboBox->addItem("83: General Purpose Button 4", 85);
    ccComboBox->addItem("84:", 86);
    ccComboBox->addItem("85:", 87);
    ccComboBox->addItem("86:", 88);
    ccComboBox->addItem("87:", 89);
    ccComboBox->addItem("88:", 90);
    ccComboBox->addItem("89:", 91);
    ccComboBox->addItem("90:", 92);
    ccComboBox->addItem("91: Effects Level", 93);
    ccComboBox->addItem("92: Tremolo Level", 94);
    ccComboBox->addItem("93: Chorus Level", 95);
    ccComboBox->addItem("94: Celeste Level", 96);
    ccComboBox->addItem("95: Phaser Level", 97);
    ccComboBox->addItem("96: Data Button Inc", 98);
    ccComboBox->addItem("97: Data Button Dec", 99);
    ccComboBox->addItem("98: NRPN (fine)", 100);
    ccComboBox->addItem("99: NRPN (coarse)", 101);
    ccComboBox->addItem("100: RPN (fine)", 102);
    ccComboBox->addItem("101: RPN (coarse)", 103);
    ccComboBox->addItem("102:", 104);
    ccComboBox->addItem("103:", 105);
    ccComboBox->addItem("104:", 106);
    ccComboBox->addItem("105:", 107);
    ccComboBox->addItem("106:", 108);
    ccComboBox->addItem("107:", 109);
    ccComboBox->addItem("108:", 110);
    ccComboBox->addItem("109:", 111);
    ccComboBox->addItem("110:", 112);
    ccComboBox->addItem("111:", 113);
    ccComboBox->addItem("112:", 114);
    ccComboBox->addItem("113:", 115);
    ccComboBox->addItem("114:", 116);
    ccComboBox->addItem("115:", 117);
    ccComboBox->addItem("116:", 118);
    ccComboBox->addItem("117:", 119);
    ccComboBox->addItem("118:", 120);
    ccComboBox->addItem("119:", 121);
    ccComboBox->addItem("120: All Sound Off", 122);
    ccComboBox->addItem("121: All Controllers Off", 123);
    ccComboBox->addItem("122: Local Keyboard", 124);
    ccComboBox->addItem("123: All Notes Off", 125);
    ccComboBox->addItem("124: Omni Mode Off", 126);
    ccComboBox->addItem("125: Omni Mode On", 127);
    ccComboBox->addItem("126: Mono Operation", 128);
    ccComboBox->addItem("127: Poly Operation", 129);
    ccComboBox->addListener(this);

    latchButton = std::make_unique<ToggleButton>("latchButton");
    addAndMakeVisible(latchButton.get());
    latchButton->setButtonText("Latch CC Value");
    latchButton->addListener(this);

    slider = std::make_unique<MappingSlider>("new slider");
    addAndMakeVisible(slider.get());
    slider->setRange(0, 1, 0);
    slider->setTextBoxStyle(MappingSlider::NoTextBox, false, 80, 20);
    slider->setColour(Slider::thumbColourId, Colour(0xff9a9181));
    slider->addListener(this);

    rangeLabel = std::make_unique<Label>("rangeLabel", "Parameter Range:");
    addAndMakeVisible(rangeLabel.get());
    rangeLabel->setFont(Font(juce::FontOptions().withHeight(15.0f)));
    rangeLabel->setJustificationType(Justification::centredLeft);
    rangeLabel->setEditable(false, false, false);
    rangeLabel->setColour(TextEditor::textColourId, Colours::black);
    rangeLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    paramComboBox = std::make_unique<ComboBox>("paramComboBox");
    addAndMakeVisible(paramComboBox.get());
    paramComboBox->setEditableText(false);
    paramComboBox->setJustificationType(Justification::centredLeft);
    paramComboBox->setTextWhenNothingSelected({});
    paramComboBox->setTextWhenNoChoicesAvailable("(no choices)");
    paramComboBox->addListener(this);

    setInterceptsMouseClicks(false, true);

    ccComboBox->setSelectedId(cc + 2, true);
    latchButton->setToggleState(latch, false);
    slider->setMaxValue(upperBound, dontSendNotification);
    slider->setMinValue(lowerBound, dontSendNotification);

    rangeLabel->setInterceptsMouseClicks(false, true);

    slider->setColour(MappingSlider::thumbColourId, ColourScheme::getInstance().colours["Slider Colour"]);

    setSize(728, 400);
}

MappingEntryMidi::~MappingEntryMidi() {
    ccComboBox.reset();
    latchButton.reset();
    slider.reset();
    rangeLabel.reset();
    paramComboBox.reset();
}

void MappingEntryMidi::paint(Graphics& g) {
    g.setColour(ColourScheme::getInstance().colours["Vector Colour"]);

    g.setColour(Colour(0x80000000));
    g.strokePath(internalPath1, PathStrokeType(5.0000f, PathStrokeType::curved, PathStrokeType::rounded));
}

void MappingEntryMidi::resized() {
    ccComboBox->setBounds(8, 8, 144, 24);
    latchButton->setBounds(160, 8, 120, 24);
    slider->setBounds(440, 8, 128, 24);
    rangeLabel->setBounds(320, 8, 128, 24);
    paramComboBox->setBounds(576, 8, 144, 24);
    internalPath1.clear();
    internalPath1.startNewSubPath(298.0f, 12.0f);
    internalPath1.lineTo(304.0f, 20.0f);
    internalPath1.lineTo(298.0f, 28.0f);
}

void MappingEntryMidi::comboBoxChanged(ComboBox* comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == ccComboBox.get()) {
        int selected = ccComboBox->getSelectedId();

        if (selected == 1) {
            midiLearn = true;
            mappingsDialog->activateMidiLearn(index);
        } else {
            if (midiLearn) {
                mappingsDialog->deactivateMidiLearn(index);
                midiLearn = false;
            }
            mappingsDialog->setCc(index, selected - 2);
        }
    } else if (comboBoxThatHasChanged == paramComboBox.get()) {
        mappingsDialog->setParameter(index, paramComboBox->getSelectedId() - 1);
    }
}

void MappingEntryMidi::buttonClicked(Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == latchButton.get()) {
        mappingsDialog->setLatch(index, latchButton->getToggleState());
    }
}

void MappingEntryMidi::sliderValueChanged(MappingSlider* sliderThatWasMoved) {
    if (sliderThatWasMoved == slider.get()) {
        mappingsDialog->setLowerBound(index, static_cast<float>(slider->getMinValue()));
        mappingsDialog->setUpperBound(index, static_cast<float>(slider->getMaxValue()));
    }
}

void MappingEntryMidi::addParameter(const String& param) {
    String tempstr = param;

    if (tempstr.isEmpty())
        tempstr = "<no name>";
    paramComboBox->addItem(tempstr, paramComboBox->getNumItems() + 1);
}

void MappingEntryMidi::selectParameter(int index) {
    paramComboBox->setSelectedId(index + 1, true);
}
