// LooperEditor.h - Full editor for the looper processor.
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

#ifndef LOOPEREDITOR_H_
#define LOOPEREDITOR_H_

#include <JuceHeader.h>
#include <memory>

#include "WaveformDisplay.h"

class LooperProcessor;

/// Full editor for the looper processor.
class LooperEditor : public AudioProcessorEditor,
                     public FilenameComponentListener,
                     public Timer,
                     public ChangeListener,
                     public juce::Button::Listener,
                     public juce::Label::Listener,
                     public juce::Slider::Listener
{
  public:
    /// Constructor.
    LooperEditor(LooperProcessor* proc, AudioThumbnail* thumbnail);
    /// Destructor.
    ~LooperEditor() override;

    /// Called when the user selects a sound file.
    void filenameComponentChanged(FilenameComponent* filenameComp) override;
    /// Called on each timer tick to update the read position.
    void timerCallback() override;
    /// Called when the waveform display or processor broadcasts a change.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    /// Changes the waveform display background colour.
    void setWaveformBackground(const Colour& col);
    /// Clears the waveform display.
    void clearDisplay();

    void paint(Graphics& g) override;
    void resized() override;
    void buttonClicked(Button* buttonThatWasClicked) override;
    void labelTextChanged(Label* labelThatHasChanged) override;
    void sliderValueChanged(Slider* sliderThatWasMoved) override;

  private:
    /// Associated looper processor.
    LooperProcessor* processor;

    /// SVG images used for the play/pause button states.
    std::unique_ptr<Drawable> playImage;
    std::unique_ptr<Drawable> pauseImage;
    /// Whether the play/pause button is showing the play icon.
    bool playing;

    /// SVG images used for the record button states.
    std::unique_ptr<Drawable> recordImage;
    std::unique_ptr<Drawable> stopImage;
    /// Whether the record button is showing the record icon.
    bool recording;

    /// Waveform display for the loop.
    std::unique_ptr<WaveformDisplay> fileDisplay;
    /// Filename selector.
    std::unique_ptr<FilenameComponent> filename;
    /// Sync to main transport toggle.
    std::unique_ptr<ToggleButton> syncButton;
    /// Stop recording after a bar toggle.
    std::unique_ptr<ToggleButton> stopAfterBarButton;
    /// Play/pause button.
    std::unique_ptr<DrawableButton> playPauseButton;
    /// Return-to-zero button.
    std::unique_ptr<DrawableButton> rtzButton;
    /// Record button.
    std::unique_ptr<DrawableButton> recordButton;
    /// Autoplay toggle.
    std::unique_ptr<ToggleButton> autoPlayButton;
    /// Bar length label.
    std::unique_ptr<Label> barLengthLabel;
    /// Time signature separator.
    std::unique_ptr<Label> separatorLabel;
    /// Time signature numerator.
    std::unique_ptr<Label> numeratorLabel;
    /// Time signature denominator.
    std::unique_ptr<Label> denominatorLabel;
    /// Loop level label.
    std::unique_ptr<Label> loopLevelLabel;
    /// Loop level slider.
    std::unique_ptr<Slider> loopLevelSlider;
    /// Input level label.
    std::unique_ptr<Label> inputLevelLabel;
    /// Input level slider.
    std::unique_ptr<Slider> inputLevelSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LooperEditor)
};

#endif
