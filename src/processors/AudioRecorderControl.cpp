// AudioRecorderControl.cpp - UI control for the audio recorder processor.
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

#include "AudioRecorderControl.h"

#include "ColourScheme.h"
#include "FilePlayerControl.h"
#include "JuceHelperStuff.h"
#include "PedalboardProcessors.h"
#include "Vectors.h"
#include "WaveformDisplay.h"

AudioRecorderControl::AudioRecorderControl(RecorderProcessor* proc, AudioThumbnail& thumbnail)
    : processor(proc), recording(false) {
    fileDisplay = std::make_unique<WaveformDisplayLite>(thumbnail);
    addAndMakeVisible(*fileDisplay);
    fileDisplay->setName("fileDisplay");

    filename =
        std::make_unique<FilenameComponent>("filename", File(), true, false, true, "*.wav", ".wav", "<no file loaded>");
    addAndMakeVisible(*filename);
    filename->setName("filename");

    syncButton = std::make_unique<ToggleButton>("syncButton");
    addAndMakeVisible(*syncButton);
    syncButton->setButtonText("Sync to main transport");
    syncButton->addListener(this);

    recordButton = std::make_unique<DrawableButton>("recordButton", DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*recordButton);
    recordButton->setName("recordButton");

    recordImage.reset(JuceHelperStuff::loadSVGFromMemory(Vectors::recordbutton_svg, Vectors::recordbutton_svgSize));
    stopImage.reset(JuceHelperStuff::loadSVGFromMemory(Vectors::stopbutton_svg, Vectors::stopbutton_svgSize));
    recordButton->setImages(recordImage.get());
    recordButton->setColour(DrawableButton::backgroundColourId, ColourScheme::getInstance().colours["Button Colour"]);
    recordButton->setColour(DrawableButton::backgroundOnColourId, ColourScheme::getInstance().colours["Button Colour"]);
    recordButton->addListener(this);
    recordButton->setTooltip("Record audio input");

    const File& soundFile = processor->getFile();
    if (soundFile != File())
        filename->setCurrentFile(soundFile, true, dontSendNotification);
    else
        filename->setDefaultBrowseTarget(FilePlayerControl::lastDir);

    syncButton->setToggleState(processor->getParameter(RecorderProcessor::SyncToMainTransport) > 0.5f, false);

    filename->addListener(this);
    processor->addChangeListener(this);

    setSize(300, 100);
}

AudioRecorderControl::~AudioRecorderControl() {
    processor->removeChangeListener(this);
    removeAllChildren();
}

void AudioRecorderControl::paint(Graphics& /*g*/) {}

void AudioRecorderControl::resized() {
    fileDisplay->setBounds(0, 28, getWidth() - 2, getHeight() - 48);
    filename->setBounds(0, 0, getWidth() - 28, 24);
    syncButton->setBounds(0, getHeight() - 23, 168, 24);
    recordButton->setBounds(getWidth() - 26, 0, 24, 24);
}

void AudioRecorderControl::buttonClicked(Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == syncButton.get()) {
        bool val = syncButton->getToggleState();
        processor->setParameter(RecorderProcessor::SyncToMainTransport, val ? 1.0f : 0.0f);
    } else if (buttonThatWasClicked == recordButton.get()) {
        if (!recording)
            recordButton->setImages(stopImage.get());
        else
            recordButton->setImages(recordImage.get());
        recording = !recording;

        processor->setParameter(RecorderProcessor::Record, 1.0f);
    }
}

void AudioRecorderControl::filenameComponentChanged(FilenameComponent* filenameComp) {
    File phil = filenameComp->getCurrentFile();
    processor->cacheFile(phil);
    FilePlayerControl::lastDir = phil.getParentDirectory();
}

void AudioRecorderControl::changeListenerCallback(ChangeBroadcaster* source) {
    if (source == processor) {
        if (processor->isRecording()) {
            recordButton->setImages(stopImage.get());
            recording = true;
        } else {
            recordButton->setImages(recordImage.get());
            recording = false;
        }

        syncButton->setToggleState(processor->getParameter(RecorderProcessor::SyncToMainTransport) > 0.5f, false);
        filename->setCurrentFile(processor->getFile(), true, dontSendNotification);
    }
}

void AudioRecorderControl::setWaveformBackground(const Colour& col) {
    fileDisplay->setBackgroundColour(col);
}
