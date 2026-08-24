// LogDisplay.cpp - Component for displaying log events.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2013 Niall Moody.
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

#include "LogDisplay.h"

#include "LogFile.h"

LogDisplay::LogDisplay() {
    logEditor = std::make_unique<juce::TextEditor>("logEditor");
    addAndMakeVisible(*logEditor);
    logEditor->setMultiLine(true);
    logEditor->setReturnKeyStartsNewLine(true);
    logEditor->setReadOnly(true);
    logEditor->setScrollbarsShown(true);
    logEditor->setCaretVisible(false);
    logEditor->setPopupMenuEnabled(true);
    logEditor->setText(juce::String());

    startStopButton = std::make_unique<juce::TextButton>("startStopButton");
    addAndMakeVisible(*startStopButton);
    startStopButton->setButtonText("Start Logging");
    startStopButton->addListener(this);

    midiButton = std::make_unique<juce::ToggleButton>("midiButton");
    addAndMakeVisible(*midiButton);
    midiButton->setButtonText("MIDI");
    midiButton->addListener(this);
    midiButton->setToggleState(true, false);

    oscButton = std::make_unique<juce::ToggleButton>("oscButton");
    addAndMakeVisible(*oscButton);
    oscButton->setButtonText("OSC");
    oscButton->addListener(this);
    oscButton->setToggleState(true, false);

    pedalboardButton = std::make_unique<juce::ToggleButton>("pedalboardButton");
    addAndMakeVisible(*pedalboardButton);
    pedalboardButton->setButtonText("Pedalboard");
    pedalboardButton->addListener(this);
    pedalboardButton->setToggleState(true, false);

    filterLabel = std::make_unique<juce::Label>("filterLabel", "Filter:");
    addAndMakeVisible(*filterLabel);
    filterLabel->setFont(juce::Font(juce::FontOptions().withHeight(15.0f)));
    filterLabel->setJustificationType(juce::Justification::centredLeft);
    filterLabel->setEditable(false, false, false);
    filterLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    filterLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x0));

    if (LogFile::getInstance().getIsLogging()) {
        updateLog(true);

        startStopButton->setButtonText("Stop Logging");
    }

    LogFile::getInstance().addChangeListener(this);

    setSize(600, 400);
}

LogDisplay::~LogDisplay() {
    LogFile::getInstance().removeChangeListener(this);
}

void LogDisplay::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xffeeece1));
}

void LogDisplay::resized() {
    logEditor->setBounds(8, 8, getWidth() - 16, getHeight() - 40);
    startStopButton->setBounds(8, getHeight() - 28, 150, 24);
    midiButton->setBounds(208, getHeight() - 28, 56, 24);
    oscButton->setBounds(264, getHeight() - 28, 56, 24);
    pedalboardButton->setBounds(320, getHeight() - 28, 96, 24);
    filterLabel->setBounds(164, getHeight() - 28, 48, 24);
}

void LogDisplay::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == startStopButton.get()) {
        if (LogFile::getInstance().getIsLogging()) {
            LogFile::getInstance().stop();
            logEditor->setText("");
            lastEvent = juce::Time();

            startStopButton->setButtonText("Start Logging");
        } else {
            LogFile::getInstance().start();

            startStopButton->setButtonText("Stop Logging");
        }
    } else if (buttonThatWasClicked == midiButton.get()) {
        updateLog(true);
    } else if (buttonThatWasClicked == oscButton.get()) {
        updateLog(true);
    } else if (buttonThatWasClicked == pedalboardButton.get()) {
        updateLog(true);
    }
}

void LogDisplay::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &(LogFile::getInstance()))
        updateLog(false);
}

void LogDisplay::updateLog(bool fromTheBeginning) {
    juce::StringArray tempArr;

    if (fromTheBeginning) {
        lastEvent = juce::Time();
        logEditor->setText("");
    }
    // Make sure the caret is at the end of the string.
    else
        logEditor->moveCaretToEnd();

    // Fill out tempArr with the event types we are interested in.
    if (midiButton->getToggleState())
        tempArr.add("MIDI");
    if (oscButton->getToggleState())
        tempArr.add("OSC");
    if (pedalboardButton->getToggleState())
        tempArr.add("Pedalboard");

    // Append the new events to our editor.
    if (fromTheBeginning)
        logEditor->setText(LogFile::getInstance().getLogContents(tempArr, lastEvent));
    else
        logEditor->insertTextAtCaret(LogFile::getInstance().getLogContents(tempArr, lastEvent));

    // So that the next time this gets called getLogContents() does not
    // return the last line a second time.
    lastEvent += juce::RelativeTime(0.001);
}
