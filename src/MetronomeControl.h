// MetronomeControl.h - UI control for the metronome processor.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2026 Niall Moody.
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
//

#ifndef METRONOMECONTROL_H_
#define METRONOMECONTROL_H_

#include <JuceHeader.h>
#include <memory>

class MetronomeProcessor;

//------------------------------------------------------------------------------
/// UI control for the metronome processor.
class MetronomeControl : public Component,
                         public FilenameComponentListener,
                         public ChangeListener,
                         public juce::Button::Listener,
                         public juce::Label::Listener
{
  public:
    /// Constructor.
    /// @param editors If false, the file selectors are hidden for use on the plugin surface.
    MetronomeControl(MetronomeProcessor* proc, bool editors);
    /// Destructor.
    ~MetronomeControl() override;

    /// Called when the user selects an accent or click sound file.
    void filenameComponentChanged(FilenameComponent* fileComponentThatHasChanged) override;
    /// Called when the processor broadcasts a change.
    void changeListenerCallback(ChangeBroadcaster* source) override;

    void paint(Graphics& g) override;
    void resized() override;
    void buttonClicked(Button* buttonThatWasClicked) override;
    void labelTextChanged(Label* labelThatHasChanged) override;

  private:
    /// Associated metronome processor.
    MetronomeProcessor* processor;

    /// SVG images used for the play/pause button states.
    std::unique_ptr<Drawable> playImage;
    std::unique_ptr<Drawable> pauseImage;
    /// Whether the metronome is currently playing.
    bool playing;
    /// Whether to show the file selectors.
    bool showFileEditors;

    /// Sync to main transport toggle.
    std::unique_ptr<ToggleButton> syncButton;
    /// Play/pause button.
    std::unique_ptr<DrawableButton> playPauseButton;
    /// Accent sound file selector.
    std::unique_ptr<FilenameComponent> accentFile;
    /// Accent label.
    std::unique_ptr<Label> accentLabel;
    /// Click sound file selector.
    std::unique_ptr<FilenameComponent> clickFile;
    /// Click label.
    std::unique_ptr<Label> clickLabel;
    /// Time signature numerator.
    std::unique_ptr<Label> numeratorLabel;
    /// Time signature denominator.
    std::unique_ptr<Label> denominatorLabel;
    /// Time signature separator.
    std::unique_ptr<Label> separatorLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MetronomeControl)
};

#endif
