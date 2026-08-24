// AudioRecorderControl.h - UI control for the audio recorder processor.
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

#ifndef AUDIORECORDERCONTROL_H_
#define AUDIORECORDERCONTROL_H_

#include <JuceHeader.h>
#include <memory>

class RecorderProcessor;
class WaveformDisplayLite;

/// UI control for the audio recorder processor.
class AudioRecorderControl : public Component,
                             public FilenameComponentListener,
                             public ChangeListener,
                             public juce::Button::Listener
{
  public:
    AudioRecorderControl(RecorderProcessor* proc, AudioThumbnail& thumbnail);
    ~AudioRecorderControl() override;

    /// Called when the user selects a sound file.
    void filenameComponentChanged(FilenameComponent* filenameComp) override;
    /// Called when the processor broadcasts a change.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Changes the waveform display background colour.
    void setWaveformBackground(const Colour& col);

    void paint(Graphics& g) override;
    void resized() override;
    void buttonClicked(Button* buttonThatWasClicked) override;

  private:
    /// Associated audio recorder processor.
    RecorderProcessor* processor;

    /// SVG images used for the record button states.
    std::unique_ptr<Drawable> recordImage;
    std::unique_ptr<Drawable> stopImage;
    /// Whether the record button is currently showing the record icon.
    bool recording;

    /// Waveform display for the recorded file.
    std::unique_ptr<WaveformDisplayLite> fileDisplay;
    /// Filename selector.
    std::unique_ptr<FilenameComponent> filename;
    /// Sync to main transport toggle.
    std::unique_ptr<ToggleButton> syncButton;
    /// Record toggle button.
    std::unique_ptr<DrawableButton> recordButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioRecorderControl)
};

#endif
