// PreferencesDialog.h - Preferences dialog for audio, MIDI, OSC, and other options.
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

#ifndef PREFERENCESDIALOG_H_
#define PREFERENCESDIALOG_H_

#include <JuceHeader.h>

class MainPanel;

/// Dialog for configuring application preferences.
///
/// Contains settings for OSC port and multicast address, visible I/O nodes,
/// MIDI options, and other options such as tray icon, fixed-size windows,
/// and audio settings persistence.
class PreferencesDialog : public Component,
                          public TextEditor::Listener,
                          public juce::Button::Listener
{
  public:
    PreferencesDialog(MainPanel* panel, const String& port, const String& multicastAddress);
    ~PreferencesDialog() override;

    /// Not used.
    void textEditorTextChanged(TextEditor& editor) override { (void) editor; }
    /// Used to update the mapping's OSC address.
    void textEditorReturnKeyPressed(TextEditor& editor) override;
    /// Used to revert any changes the user made.
    void textEditorEscapeKeyPressed(TextEditor& editor) override;
    /// Used to update the mapping's OSC address.
    void textEditorFocusLost(TextEditor& editor) override;

    void paint(Graphics& g) override;
    void resized() override;
    void buttonClicked(Button* buttonThatWasClicked) override;

  private:
    /// The app's MainPanel.
    MainPanel* mainPanel;
    /// The current port to listen on.
    String currentPort;
    /// The current multicast address to listen on.
    String currentMulticast;

    std::unique_ptr<Label> oscPortLabel;
    std::unique_ptr<TextEditor> oscPortEditor;
    std::unique_ptr<Label> oscLabel;
    std::unique_ptr<Label> oscMulticastLabel;
    std::unique_ptr<TextEditor> oscMulticastEditor;
    std::unique_ptr<Label> multicastHintLabel;
    std::unique_ptr<Label> ioOptionsLabel;
    std::unique_ptr<ToggleButton> audioInputButton;
    std::unique_ptr<ToggleButton> midiInputButton;
    std::unique_ptr<ToggleButton> oscInputButton;
    std::unique_ptr<Label> otherLabel;
    std::unique_ptr<ToggleButton> mappingsWindowButton;
    std::unique_ptr<ToggleButton> loopPatchesButton;
    std::unique_ptr<ToggleButton> windowsOnTopButton;
    std::unique_ptr<ToggleButton> ignorePinNamesButton;
    std::unique_ptr<Label> midiLabel;
    std::unique_ptr<ToggleButton> midiProgramChangeButton;
    std::unique_ptr<ToggleButton> mmcTransportButton;
    std::unique_ptr<ToggleButton> useTrayIconButton;
    std::unique_ptr<ToggleButton> startInTrayButton;
    std::unique_ptr<ToggleButton> fixedSizeButton;
    std::unique_ptr<ToggleButton> pdlAudioSettingsButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreferencesDialog)
};

#endif
