// PreferencesDialog.cpp - Preferences dialog for audio, MIDI, OSC, and other options.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2012 Niall Moody.
// Copyright (c) 2026 Pedalboard3 Project.
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

#include "MainPanel.h"
#include "PropertiesSingleton.h"
#include "ColourScheme.h"
#include "App.h"

#include "PreferencesDialog.h"

//==============================================================================
PreferencesDialog::PreferencesDialog(MainPanel* panel, const String& port, const String& multicastAddress)
{
    oscPortLabel = std::make_unique<Label>("oscPortLabel",
                                                             "OSC Port:");
    addAndMakeVisible(*oscPortLabel);
    oscPortLabel->setFont(juce::FontOptions().withHeight(15.0f));
    oscPortLabel->setJustificationType(Justification::centredLeft);
    oscPortLabel->setEditable(false, false, false);
    oscPortLabel->setColour(TextEditor::textColourId, Colours::black);
    oscPortLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    oscPortEditor = std::make_unique<TextEditor>("oscPortEditor");
    addAndMakeVisible(*oscPortEditor);
    oscPortEditor->setMultiLine(false);
    oscPortEditor->setReturnKeyStartsNewLine(false);
    oscPortEditor->setReadOnly(false);
    oscPortEditor->setScrollbarsShown(true);
    oscPortEditor->setCaretVisible(true);
    oscPortEditor->setPopupMenuEnabled(true);
    oscPortEditor->setText("5678");

    oscLabel = std::make_unique<Label>("oscLabel",
                                                         "Open Sound Control Options");
    addAndMakeVisible(*oscLabel);
    oscLabel->setFont(juce::FontOptions().withHeight(15.0f).withStyle("Bold"));
    oscLabel->setJustificationType(Justification::centredLeft);
    oscLabel->setEditable(false, false, false);
    oscLabel->setColour(TextEditor::textColourId, Colours::black);
    oscLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    oscMulticastLabel = std::make_unique<Label>("oscMulticastLabel",
                                                                  "OSC Multicast Address:");
    addAndMakeVisible(*oscMulticastLabel);
    oscMulticastLabel->setFont(juce::FontOptions().withHeight(15.0f));
    oscMulticastLabel->setJustificationType(Justification::centredLeft);
    oscMulticastLabel->setEditable(false, false, false);
    oscMulticastLabel->setColour(TextEditor::textColourId, Colours::black);
    oscMulticastLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    oscMulticastEditor = std::make_unique<TextEditor>("oscMulticastEditor");
    addAndMakeVisible(*oscMulticastEditor);
    oscMulticastEditor->setMultiLine(false);
    oscMulticastEditor->setReturnKeyStartsNewLine(false);
    oscMulticastEditor->setReadOnly(false);
    oscMulticastEditor->setScrollbarsShown(true);
    oscMulticastEditor->setCaretVisible(true);
    oscMulticastEditor->setPopupMenuEnabled(true);
    oscMulticastEditor->setText({});

    multicastHintLabel = std::make_unique<Label>("multicastHintLabel",
                                                                   "(leave blank for a one-to-one connection)");
    addAndMakeVisible(*multicastHintLabel);
    multicastHintLabel->setFont(juce::FontOptions().withHeight(15.0f));
    multicastHintLabel->setJustificationType(Justification::centredLeft);
    multicastHintLabel->setEditable(false, false, false);
    multicastHintLabel->setColour(Label::textColourId, Colour(0x80000000));
    multicastHintLabel->setColour(TextEditor::textColourId, Colours::black);
    multicastHintLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    ioOptionsLabel = std::make_unique<Label>("ioOptionsLabel",
                                                               "Visible I/O Nodes");
    addAndMakeVisible(*ioOptionsLabel);
    ioOptionsLabel->setFont(juce::FontOptions().withHeight(15.0f).withStyle("Bold"));
    ioOptionsLabel->setJustificationType(Justification::centredLeft);
    ioOptionsLabel->setEditable(false, false, false);
    ioOptionsLabel->setColour(TextEditor::textColourId, Colours::black);
    ioOptionsLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    audioInputButton = std::make_unique<ToggleButton>("audioInputButton");
    addAndMakeVisible(*audioInputButton);
    audioInputButton->setButtonText("Audio Input");
    audioInputButton->addListener(this);
    audioInputButton->setToggleState(true, false);

    midiInputButton = std::make_unique<ToggleButton>("midiInputButton");
    addAndMakeVisible(*midiInputButton);
    midiInputButton->setButtonText("Midi Input");
    midiInputButton->addListener(this);
    midiInputButton->setToggleState(true, false);

    oscInputButton = std::make_unique<ToggleButton>("oscInputButton");
    addAndMakeVisible(*oscInputButton);
    oscInputButton->setButtonText("OSC Input");
    oscInputButton->addListener(this);
    oscInputButton->setToggleState(true, false);

    otherLabel = std::make_unique<Label>("otherLabel",
                                                           "Other Options");
    addAndMakeVisible(*otherLabel);
    otherLabel->setFont(juce::FontOptions().withHeight(15.0f).withStyle("Bold"));
    otherLabel->setJustificationType(Justification::centredLeft);
    otherLabel->setEditable(false, false, false);
    otherLabel->setColour(TextEditor::textColourId, Colours::black);
    otherLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    mappingsWindowButton = std::make_unique<ToggleButton>("mappingsWindowButton");
    addAndMakeVisible(*mappingsWindowButton);
    mappingsWindowButton->setButtonText("Open mappings window on successful param connection");
    mappingsWindowButton->addListener(this);
    mappingsWindowButton->setToggleState(true, false);

    loopPatchesButton = std::make_unique<ToggleButton>("loopPatchesButton");
    addAndMakeVisible(*loopPatchesButton);
    loopPatchesButton->setButtonText("Loop next/prev patch action");
    loopPatchesButton->addListener(this);
    loopPatchesButton->setToggleState(true, false);

    windowsOnTopButton = std::make_unique<ToggleButton>("windowsOnTopButton");
    addAndMakeVisible(*windowsOnTopButton);
    windowsOnTopButton->setButtonText("Set plugin windows Always On Top");
    windowsOnTopButton->addListener(this);

    ignorePinNamesButton = std::make_unique<ToggleButton>("ignorePinNamesButton");
    addAndMakeVisible(*ignorePinNamesButton);
    ignorePinNamesButton->setButtonText("Ignore plugin pin names");
    ignorePinNamesButton->addListener(this);

    midiLabel = std::make_unique<Label>("midiLabel",
                                                          "Midi Options");
    addAndMakeVisible(*midiLabel);
    midiLabel->setFont(juce::FontOptions().withHeight(15.0f).withStyle("Bold"));
    midiLabel->setJustificationType(Justification::centredLeft);
    midiLabel->setEditable(false, false, false);
    midiLabel->setColour(TextEditor::textColourId, Colours::black);
    midiLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    midiProgramChangeButton = std::make_unique<ToggleButton>("midiProgramChangeButton");
    addAndMakeVisible(*midiProgramChangeButton);
    midiProgramChangeButton->setButtonText("Program Change messages switch patches");
    midiProgramChangeButton->addListener(this);

    mmcTransportButton = std::make_unique<ToggleButton>("mmcTransportButton");
    addAndMakeVisible(*mmcTransportButton);
    mmcTransportButton->setButtonText("Main transport responds to MMC");
    mmcTransportButton->addListener(this);

    useTrayIconButton = std::make_unique<ToggleButton>("useTrayIconButton");
    addAndMakeVisible(*useTrayIconButton);
    useTrayIconButton->setButtonText("Display tray icon (not OSX)");
    useTrayIconButton->addListener(this);

    startInTrayButton = std::make_unique<ToggleButton>("startInTrayButton");
    addAndMakeVisible(*startInTrayButton);
    startInTrayButton->setButtonText("Start in tray (not OSX)");
    startInTrayButton->addListener(this);

    fixedSizeButton = std::make_unique<ToggleButton>("fixedSizeButton");
    addAndMakeVisible(*fixedSizeButton);
    fixedSizeButton->setButtonText("Force fixed-size plugin windows");
    fixedSizeButton->addListener(this);
    fixedSizeButton->setToggleState(true, false);

    pdlAudioSettingsButton = std::make_unique<ToggleButton>("pdlAudioSettingsButton");
    addAndMakeVisible(*pdlAudioSettingsButton);
    pdlAudioSettingsButton->setButtonText("Save audio settings in .pdl files");
    pdlAudioSettingsButton->addListener(this);

    bool useTrayIcon;

    mainPanel = panel;
    currentPort = port;
    currentMulticast = multicastAddress;

    oscPortEditor->setText(currentPort);
    oscMulticastEditor->setText(currentMulticast);

    oscPortEditor->addListener(this);
    oscMulticastEditor->addListener(this);

    audioInputButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("AudioInput", true), false);
    midiInputButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("MidiInput", true), false);
    oscInputButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("OscInput", true), false);

    midiProgramChangeButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("midiProgramChange", false), false);
    mmcTransportButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("mmcTransport", false), false);

    mappingsWindowButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("AutoMappingsWindow", true), false);
    loopPatchesButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("LoopPatches", true), false);
    windowsOnTopButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("WindowsOnTop", false), false);
    ignorePinNamesButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("IgnorePinNames", false), false);

    fixedSizeButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("fixedSizeWindows", true), false);

    pdlAudioSettingsButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("pdlAudioSettings", false), false);

#ifndef JUCE_MAC
    useTrayIcon = PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("useTrayIcon", false);
    useTrayIconButton->setToggleState(useTrayIcon, false);
    if (useTrayIcon)
        startInTrayButton->setToggleState(PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("startInTray", false), false);
    else
    {
        startInTrayButton->setToggleState(false, false);
        startInTrayButton->setEnabled(false);
    }
#else
    useTrayIconButton->setEnabled(false);
    startInTrayButton->setEnabled(false);
#endif

    oscPortLabel->setColour(TextEditor::textColourId,
                            ColourScheme::getInstance().colours["Text Colour"]);
    oscLabel->setColour(TextEditor::textColourId,
                        ColourScheme::getInstance().colours["Text Colour"]);
    oscMulticastLabel->setColour(TextEditor::textColourId,
                                 ColourScheme::getInstance().colours["Text Colour"]);
    multicastHintLabel->setColour(TextEditor::textColourId,
                                  ColourScheme::getInstance().colours["Text Colour"]);
    ioOptionsLabel->setColour(TextEditor::textColourId,
                              ColourScheme::getInstance().colours["Text Colour"]);
    otherLabel->setColour(TextEditor::textColourId,
                          ColourScheme::getInstance().colours["Text Colour"]);

    setSize(560, 530);
}

//------------------------------------------------------------------------------

PreferencesDialog::~PreferencesDialog()
{
}

//------------------------------------------------------------------------------

void PreferencesDialog::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);

    g.setColour(ColourScheme::getInstance().colours["Dialog Inner Background"]);
    g.fillRect(12, 132, getWidth() - 24, 82);

    g.setColour(Colour(0x40000000));
    g.drawRect(12, 132, getWidth() - 24, 82, 1);
}

//------------------------------------------------------------------------------

void PreferencesDialog::resized()
{
    oscPortLabel->setBounds(8, 40, 72, 24);
    oscPortEditor->setBounds(80, 40, 64, 24);
    oscLabel->setBounds(0, 8, 208, 24);
    oscMulticastLabel->setBounds(8, 72, 160, 24);
    oscMulticastEditor->setBounds(168, 72, 112, 24);
    multicastHintLabel->setBounds(280, 72, 272, 24);
    ioOptionsLabel->setBounds(0, 104, 136, 24);
    audioInputButton->setBounds(16, 136, 96, 24);
    midiInputButton->setBounds(16, 160, 88, 24);
    oscInputButton->setBounds(16, 184, 88, 24);
    midiLabel->setBounds(0, 224, 104, 24);
    midiProgramChangeButton->setBounds(16, 248, 288, 24);
    mmcTransportButton->setBounds(16, 272, 232, 24);
    otherLabel->setBounds(0, 304, 150, 24);
    mappingsWindowButton->setBounds(16, 328, 376, 24);
    loopPatchesButton->setBounds(16, 352, 208, 24);
    windowsOnTopButton->setBounds(16, 376, 256, 24);
    ignorePinNamesButton->setBounds(16, 400, 176, 24);
    useTrayIconButton->setBounds(16, 424, 200, 24);
    startInTrayButton->setBounds(16, 448, 168, 24);
    fixedSizeButton->setBounds(16, 472, 224, 24);
    pdlAudioSettingsButton->setBounds(16, 496, 224, 24);
}

//------------------------------------------------------------------------------

void PreferencesDialog::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == audioInputButton.get())
    {
        mainPanel->enableAudioInput(audioInputButton->getToggleState());
    }
    else if (buttonThatWasClicked == midiInputButton.get())
    {
        mainPanel->enableMidiInput(midiInputButton->getToggleState());
    }
    else if (buttonThatWasClicked == oscInputButton.get())
    {
        mainPanel->enableOscInput(oscInputButton->getToggleState());
    }
    else if (buttonThatWasClicked == mappingsWindowButton.get())
    {
        mainPanel->setAutoMappingsWindow(mappingsWindowButton->getToggleState());
    }
    else if (buttonThatWasClicked == loopPatchesButton.get())
    {
        PropertiesSingleton::getInstance().getUserSettings()->setValue("LoopPatches",
                                                                       loopPatchesButton->getToggleState());
    }
    else if (buttonThatWasClicked == windowsOnTopButton.get())
    {
        PropertiesSingleton::getInstance().getUserSettings()->setValue("WindowsOnTop",
                                                                       windowsOnTopButton->getToggleState());
    }
    else if (buttonThatWasClicked == ignorePinNamesButton.get())
    {
        PropertiesSingleton::getInstance().getUserSettings()->setValue("IgnorePinNames",
                                                                       ignorePinNamesButton->getToggleState());
    }
    else if (buttonThatWasClicked == midiProgramChangeButton.get())
    {
        PropertiesSingleton::getInstance().getUserSettings()->setValue("midiProgramChange",
                                                                       midiProgramChangeButton->getToggleState());
    }
    else if (buttonThatWasClicked == mmcTransportButton.get())
    {
        PropertiesSingleton::getInstance().getUserSettings()->setValue("mmcTransport",
                                                                       mmcTransportButton->getToggleState());
    }
    else if (buttonThatWasClicked == useTrayIconButton.get())
    {
        auto* app = dynamic_cast<Pedalboard3App*>(JUCEApplication::getInstance());
        (void) app;
        // TODO: App::showTrayIcon() will be implemented when App is fully ported.
        if (useTrayIconButton->getToggleState())
            startInTrayButton->setEnabled(true);
        else
        {
            startInTrayButton->setToggleState(false, false);
            startInTrayButton->setEnabled(false);
        }

        PropertiesSingleton::getInstance().getUserSettings()->setValue("useTrayIcon",
                                                                       useTrayIconButton->getToggleState());
    }
    else if (buttonThatWasClicked == startInTrayButton.get())
    {
        PropertiesSingleton::getInstance().getUserSettings()->setValue("startInTray",
                                                                       startInTrayButton->getToggleState());
    }
    else if (buttonThatWasClicked == fixedSizeButton.get())
    {
        PropertiesSingleton::getInstance().getUserSettings()->setValue("fixedSizeWindows",
                                                                       fixedSizeButton->getToggleState());
    }
    else if (buttonThatWasClicked == pdlAudioSettingsButton.get())
    {
        PropertiesSingleton::getInstance().getUserSettings()->setValue("pdlAudioSettings",
                                                                       pdlAudioSettingsButton->getToggleState());
    }
}

//------------------------------------------------------------------------------

void PreferencesDialog::textEditorReturnKeyPressed(TextEditor& editor)
{
    if (editor.getName() == "oscPortEditor")
    {
        currentPort = editor.getText();
        mainPanel->setSocketPort(currentPort);
    }
    else if (editor.getName() == "oscMulticastEditor")
    {
        currentMulticast = editor.getText();
        mainPanel->setSocketMulticast(currentMulticast);
    }
}

//------------------------------------------------------------------------------

void PreferencesDialog::textEditorEscapeKeyPressed(TextEditor& editor)
{
    if (editor.getName() == "oscPortEditor")
        editor.setText(currentPort, false);
    else if (editor.getName() == "oscMulticastEditor")
        editor.setText(currentMulticast, false);
}

//------------------------------------------------------------------------------

void PreferencesDialog::textEditorFocusLost(TextEditor& editor)
{
    if (editor.getName() == "oscPortEditor")
    {
        currentPort = editor.getText();
        mainPanel->setSocketPort(currentPort);
    }
    else if (editor.getName() == "oscMulticastEditor")
    {
        currentMulticast = editor.getText();
        mainPanel->setSocketMulticast(currentMulticast);
    }
}