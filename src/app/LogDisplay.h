// LogDisplay.h - Component for displaying log events.
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

#pragma once

#include <JuceHeader.h>

/// Component which displays log events from the LogFile singleton.
///
/// Provides a read-only text editor showing logged events, a start/stop
/// button for logging, and toggle buttons to filter by event type
/// (MIDI, OSC, Pedalboard).
class LogDisplay : public juce::Component,
                   public juce::ChangeListener,
                   public juce::Button::Listener
{
  public:
    LogDisplay();
    ~LogDisplay() override;

    /// Called when the LogFile is updated.
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;

  private:
    /// Helper method. Updates logEditor.
    void updateLog(bool fromTheBeginning);

    /// The timestamp of the most recent log event.
    juce::Time lastEvent;

    std::unique_ptr<juce::TextEditor> logEditor;
    std::unique_ptr<juce::TextButton> startStopButton;
    std::unique_ptr<juce::ToggleButton> midiButton;
    std::unique_ptr<juce::ToggleButton> oscButton;
    std::unique_ptr<juce::ToggleButton> pedalboardButton;
    std::unique_ptr<juce::Label> filterLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogDisplay)
};
