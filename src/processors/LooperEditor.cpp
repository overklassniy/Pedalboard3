// LooperEditor.cpp - Full editor for the looper processor.
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

#include "LooperEditor.h"

#include "ColourScheme.h"
#include "JuceHelperStuff.h"
#include "LooperControl.h"
#include "PedalboardProcessors.h"
#include "Vectors.h"

LooperEditor::LooperEditor(LooperProcessor* proc, AudioThumbnail* thumbnail)
    : AudioProcessorEditor(proc), processor(proc), playing(false), recording(false) {
    fileDisplay = std::make_unique<WaveformDisplay>(thumbnail, false);
    addAndMakeVisible(*fileDisplay);
    fileDisplay->setName("fileDisplay");

    filename = std::make_unique<FilenameComponent>("filename", File(), true, false, true, "*.wav;*.aif", "",
                                                   "<no file loaded>");
    addAndMakeVisible(*filename);
    filename->setName("filename");

    syncButton = std::make_unique<ToggleButton>("syncButton");
    addAndMakeVisible(*syncButton);
    syncButton->setTooltip("Sync file playback to the main transport");
    syncButton->setButtonText("Sync to main transport");
    syncButton->addListener(this);

    stopAfterBarButton = std::make_unique<ToggleButton>("stopAfterBarButton");
    addAndMakeVisible(*stopAfterBarButton);
    stopAfterBarButton->setTooltip("Stop recording after a bar has elapsed.");
    stopAfterBarButton->setButtonText("Stop after bar");
    stopAfterBarButton->addListener(this);

    playPauseButton = std::make_unique<DrawableButton>("playPauseButton", DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*playPauseButton);
    playPauseButton->setName("playPauseButton");

    rtzButton = std::make_unique<DrawableButton>("rtzButton", DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*rtzButton);
    rtzButton->setName("rtzButton");

    recordButton = std::make_unique<DrawableButton>("recordButton", DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*recordButton);
    recordButton->setName("recordButton");

    autoPlayButton = std::make_unique<ToggleButton>("autoPlayButton");
    addAndMakeVisible(*autoPlayButton);
    autoPlayButton->setButtonText("Autoplay");
    autoPlayButton->addListener(this);

    barLengthLabel = std::make_unique<Label>("barLengthLabel", "Bar length:");
    addAndMakeVisible(*barLengthLabel);
    barLengthLabel->setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::plain)));
    barLengthLabel->setJustificationType(Justification::centredLeft);
    barLengthLabel->setEditable(false, false, false);
    barLengthLabel->setColour(TextEditor::textColourId, Colours::black);
    barLengthLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    separatorLabel = std::make_unique<Label>("separatorLabel", "/");
    addAndMakeVisible(*separatorLabel);
    separatorLabel->setFont(
        juce::Font(juce::FontOptions(juce::Font::getDefaultSerifFontName(), 105.0f, juce::Font::bold)));
    separatorLabel->setJustificationType(Justification::centred);
    separatorLabel->setEditable(false, false, false);
    separatorLabel->setColour(TextEditor::textColourId, Colours::black);
    separatorLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    numeratorLabel = std::make_unique<Label>("numeratorLabel", "4");
    addAndMakeVisible(*numeratorLabel);
    numeratorLabel->setFont(
        juce::Font(juce::FontOptions(juce::Font::getDefaultSerifFontName(), 105.0f, juce::Font::bold)));
    numeratorLabel->setJustificationType(Justification::centred);
    numeratorLabel->setEditable(true, true, false);
    numeratorLabel->setColour(TextEditor::textColourId, Colours::black);
    numeratorLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));
    numeratorLabel->addListener(this);

    denominatorLabel = std::make_unique<Label>("denominatorLabel", "4");
    addAndMakeVisible(*denominatorLabel);
    denominatorLabel->setFont(
        juce::Font(juce::FontOptions(juce::Font::getDefaultSerifFontName(), 105.0f, juce::Font::bold)));
    denominatorLabel->setJustificationType(Justification::centred);
    denominatorLabel->setEditable(true, true, false);
    denominatorLabel->setColour(TextEditor::textColourId, Colours::black);
    denominatorLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));
    denominatorLabel->addListener(this);

    loopLevelLabel = std::make_unique<Label>("loopLevelLabel", "Loop level");
    addAndMakeVisible(*loopLevelLabel);
    loopLevelLabel->setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::plain)));
    loopLevelLabel->setJustificationType(Justification::centred);
    loopLevelLabel->setEditable(false, false, false);
    loopLevelLabel->setColour(TextEditor::textColourId, Colours::black);
    loopLevelLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    loopLevelSlider = std::make_unique<Slider>("loopLevelSlider");
    addAndMakeVisible(*loopLevelSlider);
    loopLevelSlider->setRange(0, 1, 0);
    loopLevelSlider->setSliderStyle(Slider::RotaryVerticalDrag);
    loopLevelSlider->setTextBoxStyle(Slider::NoTextBox, false, 80, 20);
    loopLevelSlider->addListener(this);

    inputLevelLabel = std::make_unique<Label>("inputLevelLabel", "Input level");
    addAndMakeVisible(*inputLevelLabel);
    inputLevelLabel->setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::plain)));
    inputLevelLabel->setJustificationType(Justification::centred);
    inputLevelLabel->setEditable(false, false, false);
    inputLevelLabel->setColour(TextEditor::textColourId, Colours::black);
    inputLevelLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    inputLevelSlider = std::make_unique<Slider>("inputLevelSlider");
    addAndMakeVisible(*inputLevelSlider);
    inputLevelSlider->setRange(0, 1, 0);
    inputLevelSlider->setSliderStyle(Slider::RotaryVerticalDrag);
    inputLevelSlider->setTextBoxStyle(Slider::NoTextBox, false, 80, 20);
    inputLevelSlider->addListener(this);

    String tempstr;
    std::unique_ptr<Drawable> rtzImage(
        JuceHelperStuff::loadSVGFromMemory(Vectors::rtzbutton_svg, Vectors::rtzbutton_svgSize));

    playImage.reset(JuceHelperStuff::loadSVGFromMemory(Vectors::playbutton_svg, Vectors::playbutton_svgSize));
    pauseImage.reset(JuceHelperStuff::loadSVGFromMemory(Vectors::pausebutton_svg, Vectors::pausebutton_svgSize));
    playPauseButton->setImages(playImage.get());
    playPauseButton->setColour(DrawableButton::backgroundColourId,
                               ColourScheme::getInstance().colours["Button Colour"]);
    playPauseButton->setColour(DrawableButton::backgroundOnColourId,
                               ColourScheme::getInstance().colours["Button Colour"]);
    playPauseButton->addListener(this);
    playPauseButton->setTooltip("Play/pause audio file");

    recordImage.reset(JuceHelperStuff::loadSVGFromMemory(Vectors::recordbutton_svg, Vectors::recordbutton_svgSize));
    stopImage.reset(JuceHelperStuff::loadSVGFromMemory(Vectors::stopbutton_svg, Vectors::stopbutton_svgSize));
    recordButton->setImages(recordImage.get());
    recordButton->setColour(DrawableButton::backgroundColourId, ColourScheme::getInstance().colours["Button Colour"]);
    recordButton->setColour(DrawableButton::backgroundOnColourId, ColourScheme::getInstance().colours["Button Colour"]);
    recordButton->addListener(this);
    recordButton->setTooltip("Record/stop recording a loop");

    changeListenerCallback(processor);

    rtzButton->setImages(rtzImage.get());
    rtzButton->setColour(DrawableButton::backgroundColourId, ColourScheme::getInstance().colours["Button Colour"]);
    rtzButton->setColour(DrawableButton::backgroundOnColourId, ColourScheme::getInstance().colours["Button Colour"]);
    rtzButton->addListener(this);
    rtzButton->setTooltip("Return to the start of the audio file");

    const File& soundFile = processor->getFile();
    if (soundFile != File()) {
        filename->setCurrentFile(soundFile, true, dontSendNotification);
        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
    } else {
        filename->setDefaultBrowseTarget(LooperControl::lastDir);
    }

    syncButton->setToggleState(processor->getParameter(LooperProcessor::SyncToMainTransport) > 0.5f, false);
    stopAfterBarButton->setToggleState(processor->getParameter(LooperProcessor::StopAfterBar) > 0.5f, false);
    autoPlayButton->setToggleState(processor->getParameter(LooperProcessor::AutoPlay) > 0.5f, false);

    inputLevelSlider->setDoubleClickReturnValue(true, 0.5);
    inputLevelSlider->setValue(processor->getParameter(LooperProcessor::InputLevel));
    loopLevelSlider->setDoubleClickReturnValue(true, 0.5);
    loopLevelSlider->setValue(processor->getParameter(LooperProcessor::LoopLevel));

    tempstr << static_cast<int>(processor->getParameter(LooperProcessor::BarNumerator));
    numeratorLabel->setText(tempstr, dontSendNotification);
    tempstr = "";
    tempstr << static_cast<int>(processor->getParameter(LooperProcessor::BarDenominator));
    denominatorLabel->setText(tempstr, dontSendNotification);

    filename->addListener(this);
    fileDisplay->addChangeListener(this);
    processor->addChangeListener(this);

    inputLevelSlider->setColour(Slider::rotarySliderFillColourId,
                                ColourScheme::getInstance().colours["Level Dial Colour"]);
    loopLevelSlider->setColour(Slider::rotarySliderFillColourId,
                               ColourScheme::getInstance().colours["Level Dial Colour"]);

    startTimer(60);

    setSize(500, 300);
}

LooperEditor::~LooperEditor() {
    processor->removeChangeListener(this);
    removeAllChildren();
}

void LooperEditor::paint(Graphics& /*g*/) {}

void LooperEditor::resized() {
    fileDisplay->setBounds(0, 28, getWidth() - 2, getHeight() - 155);
    filename->setBounds(0, 0, getWidth() - 84, 24);
    syncButton->setBounds(0, getHeight() - 23, 168, 24);
    stopAfterBarButton->setBounds(176, getHeight() - 23, 112, 24);
    playPauseButton->setBounds(getWidth() - 80, 0, 24, 24);
    rtzButton->setBounds(getWidth() - 52, 0, 24, 24);
    recordButton->setBounds(getWidth() - 24, 0, 24, 24);
    autoPlayButton->setBounds(296, getHeight() - 23, 80, 24);
    barLengthLabel->setBounds(0, getHeight() - 127, 80, 24);
    separatorLabel->setBounds(64, getHeight() - 111, 80, 80);
    numeratorLabel->setBounds(8, getHeight() - 103, 80, 80);
    denominatorLabel->setBounds(112, getHeight() - 103, 80, 80);
    loopLevelLabel->setBounds(getWidth() - 82, getHeight() - 127, 78, 24);
    loopLevelSlider->setBounds(getWidth() - 82, getHeight() - 103, 80, 80);
    inputLevelLabel->setBounds(getWidth() - 170, getHeight() - 127, 78, 24);
    inputLevelSlider->setBounds(getWidth() - 170, getHeight() - 103, 80, 80);
}

void LooperEditor::buttonClicked(Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == syncButton.get()) {
        bool val = syncButton->getToggleState();
        processor->setParameter(LooperProcessor::SyncToMainTransport, val ? 1.0f : 0.0f);
    } else if (buttonThatWasClicked == stopAfterBarButton.get()) {
        bool val = stopAfterBarButton->getToggleState();
        processor->setParameter(LooperProcessor::StopAfterBar, val ? 1.0f : 0.0f);
    } else if (buttonThatWasClicked == autoPlayButton.get()) {
        bool val = autoPlayButton->getToggleState();
        processor->setParameter(LooperProcessor::AutoPlay, val ? 1.0f : 0.0f);
    } else if (buttonThatWasClicked == playPauseButton.get()) {
        if (!playing)
            playPauseButton->setImages(pauseImage.get());
        else
            playPauseButton->setImages(playImage.get());
        playing = !playing;
        processor->setParameter(LooperProcessor::Play, 1.0f);
    } else if (buttonThatWasClicked == rtzButton.get()) {
        processor->setParameter(LooperProcessor::ReturnToZero, 1.0f);
        fileDisplay->setReadPointer(0.0f);
    } else if (buttonThatWasClicked == recordButton.get()) {
        recording = !recording;

        if (recording) {
            if (playing) {
                playPauseButton->setImages(playImage.get());
                playing = false;
                processor->setParameter(LooperProcessor::Play, 1.0f);
            }
        }

        processor->setParameter(LooperProcessor::Record, 1.0f);
    }

    processor->sendChangeMessage();
}

void LooperEditor::labelTextChanged(Label* labelThatHasChanged) {
    if (labelThatHasChanged == numeratorLabel.get()) {
        float tempf = static_cast<float>(numeratorLabel->getText().getIntValue());
        processor->setParameter(LooperProcessor::BarNumerator, tempf);
    } else if (labelThatHasChanged == denominatorLabel.get()) {
        float tempf = static_cast<float>(denominatorLabel->getText().getIntValue());
        processor->setParameter(LooperProcessor::BarDenominator, tempf);
    }
}

void LooperEditor::sliderValueChanged(Slider* sliderThatWasMoved) {
    if (sliderThatWasMoved == loopLevelSlider.get()) {
        processor->setParameter(LooperProcessor::LoopLevel, static_cast<float>(loopLevelSlider->getValue()));
    } else if (sliderThatWasMoved == inputLevelSlider.get()) {
        processor->setParameter(LooperProcessor::InputLevel, static_cast<float>(inputLevelSlider->getValue()));
    }
}

void LooperEditor::filenameComponentChanged(FilenameComponent* filenameComp) {
    File phil = filenameComp->getCurrentFile();
    fileDisplay->setFile(phil);
    processor->setFile(phil);
    LooperControl::lastDir = phil.getParentDirectory();

    processor->sendChangeMessage();
}

void LooperEditor::timerCallback() {
    if (playing)
        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
}

void LooperEditor::changeListenerCallback(ChangeBroadcaster* source) {
    if (source == fileDisplay.get()) {
        processor->setParameter(LooperProcessor::ReadPosition, fileDisplay->getReadPointer());
        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
    } else if (source == processor) {
        if (processor->getNewFileLoaded())
            fileDisplay->setFile(processor->getFile());

        if (filename->getCurrentFile() != processor->getFile())
            filename->setCurrentFile(processor->getFile(), true, dontSendNotification);

        if (processor->isRecording()) {
            playPauseButton->setEnabled(false);

            if (!recording)
                fileDisplay->setFile(File());
            recordButton->setImages(stopImage.get());
            recording = true;
        } else {
            playPauseButton->setEnabled(true);
            recordButton->setImages(recordImage.get());
            recording = false;

            if (processor->isPlaying()) {
                playPauseButton->setImages(pauseImage.get());
                playing = true;
            } else {
                playPauseButton->setImages(playImage.get());
                playing = false;
            }
        }

        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
        syncButton->setToggleState(processor->getParameter(LooperProcessor::SyncToMainTransport) > 0.5f, false);
        stopAfterBarButton->setToggleState(processor->getParameter(LooperProcessor::StopAfterBar) > 0.5f, false);
    }
}

void LooperEditor::setWaveformBackground(const Colour& col) {
    fileDisplay->setBackgroundColour(col);
}

void LooperEditor::clearDisplay() {
    fileDisplay->setFile(File());
}
