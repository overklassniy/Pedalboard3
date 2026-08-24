// LooperControl.cpp - UI control for the looper processor.
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

#include "LooperControl.h"
#include "PedalboardProcessors.h"
#include "JuceHelperStuff.h"
#include "ColourScheme.h"
#include "Vectors.h"

File LooperControl::lastDir(File::getSpecialLocation(File::userHomeDirectory));

LooperControl::LooperControl(LooperProcessor* proc, AudioThumbnail* thumbnail)
    : processor(proc),
      playing(false),
      recording(false)
{
    fileDisplay = std::make_unique<WaveformDisplay>(thumbnail, false);
    addAndMakeVisible(*fileDisplay);
    fileDisplay->setName("fileDisplay");

    filename = std::make_unique<FilenameComponent>("filename",
                                                   File(),
                                                   true,
                                                   false,
                                                   true,
                                                   "*.wav;*.aif",
                                                   "",
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

    std::unique_ptr<Drawable> rtzImage(JuceHelperStuff::loadSVGFromMemory(Vectors::rtzbutton_svg,
                                                                          Vectors::rtzbutton_svgSize));

    playImage.reset(JuceHelperStuff::loadSVGFromMemory(Vectors::playbutton_svg,
                                                        Vectors::playbutton_svgSize));
    pauseImage.reset(JuceHelperStuff::loadSVGFromMemory(Vectors::pausebutton_svg,
                                                         Vectors::pausebutton_svgSize));
    playPauseButton->setImages(playImage.get());
    playPauseButton->setColour(DrawableButton::backgroundColourId,
                               ColourScheme::getInstance().colours["Button Colour"]);
    playPauseButton->setColour(DrawableButton::backgroundOnColourId,
                               ColourScheme::getInstance().colours["Button Colour"]);
    playPauseButton->addListener(this);
    playPauseButton->setTooltip("Play/pause audio file");

    recordImage.reset(JuceHelperStuff::loadSVGFromMemory(Vectors::recordbutton_svg,
                                                          Vectors::recordbutton_svgSize));
    stopImage.reset(JuceHelperStuff::loadSVGFromMemory(Vectors::stopbutton_svg,
                                                        Vectors::stopbutton_svgSize));
    recordButton->setImages(recordImage.get());
    recordButton->setColour(DrawableButton::backgroundColourId,
                            ColourScheme::getInstance().colours["Button Colour"]);
    recordButton->setColour(DrawableButton::backgroundOnColourId,
                            ColourScheme::getInstance().colours["Button Colour"]);
    recordButton->addListener(this);
    recordButton->setTooltip("Record/stop recording a loop");

    changeListenerCallback(processor);

    rtzButton->setImages(rtzImage.get());
    rtzButton->setColour(DrawableButton::backgroundColourId,
                         ColourScheme::getInstance().colours["Button Colour"]);
    rtzButton->setColour(DrawableButton::backgroundOnColourId,
                         ColourScheme::getInstance().colours["Button Colour"]);
    rtzButton->addListener(this);
    rtzButton->setTooltip("Return to the start of the audio file");

    const File& soundFile = processor->getFile();
    if (soundFile != File())
    {
        filename->setCurrentFile(soundFile, true, dontSendNotification);
        fileDisplay->setFile(soundFile);
        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
    }
    else
    {
        filename->setDefaultBrowseTarget(lastDir);
    }

    syncButton->setToggleState(processor->getParameter(LooperProcessor::SyncToMainTransport) > 0.5f,
                               false);
    stopAfterBarButton->setToggleState(processor->getParameter(LooperProcessor::StopAfterBar) > 0.5f,
                                       false);

    filename->addListener(this);
    fileDisplay->addChangeListener(this);
    processor->addChangeListener(this);

    startTimer(60);

    setSize(300, 100);
}

LooperControl::~LooperControl()
{
    processor->removeChangeListener(this);
    removeAllChildren();
}

void LooperControl::paint(Graphics& /*g*/)
{
}

void LooperControl::resized()
{
    fileDisplay->setBounds(0, 28, getWidth() - 2, getHeight() - 48);
    filename->setBounds(0, 0, getWidth() - 84, 24);
    syncButton->setBounds(0, getHeight() - 23, 168, 24);
    stopAfterBarButton->setBounds(176, getHeight() - 23, 112, 24);
    playPauseButton->setBounds(getWidth() - 80, 0, 24, 24);
    rtzButton->setBounds(getWidth() - 52, 0, 24, 24);
    recordButton->setBounds(getWidth() - 24, 0, 24, 24);
}

void LooperControl::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == syncButton.get())
    {
        bool val = syncButton->getToggleState();
        processor->setParameter(LooperProcessor::SyncToMainTransport,
                                val ? 1.0f : 0.0f);
    }
    else if (buttonThatWasClicked == stopAfterBarButton.get())
    {
        bool val = stopAfterBarButton->getToggleState();
        processor->setParameter(LooperProcessor::StopAfterBar,
                                val ? 1.0f : 0.0f);
    }
    else if (buttonThatWasClicked == playPauseButton.get())
    {
        if (!playing)
            playPauseButton->setImages(pauseImage.get());
        else
            playPauseButton->setImages(playImage.get());
        playing = !playing;
        processor->setParameter(LooperProcessor::Play, 1.0f);
    }
    else if (buttonThatWasClicked == rtzButton.get())
    {
        processor->setParameter(LooperProcessor::ReturnToZero, 1.0f);
        fileDisplay->setReadPointer(0.0f);
    }
    else if (buttonThatWasClicked == recordButton.get())
    {
        recording = !recording;

        if (recording)
        {
            if (playing)
            {
                playPauseButton->setImages(playImage.get());
                playing = false;
                processor->setParameter(LooperProcessor::Play, 1.0f);
            }
        }

        processor->setParameter(LooperProcessor::Record, 1.0f);
    }

    processor->sendChangeMessage();
}

void LooperControl::filenameComponentChanged(FilenameComponent* filenameComp)
{
    File phil = filenameComp->getCurrentFile();
    fileDisplay->setFile(phil);
    processor->setFile(phil);
    lastDir = phil.getParentDirectory();

    processor->sendChangeMessage();
}

void LooperControl::timerCallback()
{
    if (playing)
        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
}

void LooperControl::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == fileDisplay.get())
    {
        processor->setParameter(LooperProcessor::ReadPosition,
                                fileDisplay->getReadPointer());
        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
    }
    else if (source == processor)
    {
        if (processor->getNewFileLoaded())
            fileDisplay->setFile(processor->getFile());

        if (filename->getCurrentFile() != processor->getFile())
            filename->setCurrentFile(processor->getFile(), true, dontSendNotification);

        if (processor->isRecording())
        {
            playPauseButton->setImages(playImage.get());
            playPauseButton->setEnabled(false);

            if (!recording)
                fileDisplay->setFile(File());
            recordButton->setImages(stopImage.get());
            recording = true;
        }
        else
        {
            playPauseButton->setEnabled(true);
            recordButton->setImages(recordImage.get());
            recording = false;

            if (processor->isPlaying())
            {
                playPauseButton->setImages(pauseImage.get());
                playing = true;
            }
            else
            {
                playPauseButton->setImages(playImage.get());
                playing = false;
            }
        }

        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
        syncButton->setToggleState(processor->getParameter(LooperProcessor::SyncToMainTransport) > 0.5f,
                                   false);
        stopAfterBarButton->setToggleState(processor->getParameter(LooperProcessor::StopAfterBar) > 0.5f,
                                           false);
    }
}

void LooperControl::setWaveformBackground(const Colour& col)
{
    fileDisplay->setBackgroundColour(col);
}

void LooperControl::clearDisplay()
{
    fileDisplay->setFile(File());
}
