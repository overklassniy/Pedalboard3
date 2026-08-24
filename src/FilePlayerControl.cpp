// FilePlayerControl.cpp - UI control for the file player processor.
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

#include "FilePlayerControl.h"
#include "PedalboardProcessors.h"
#include "JuceHelperStuff.h"
#include "ColourScheme.h"
#include "Vectors.h"

//------------------------------------------------------------------------------
File FilePlayerControl::lastDir(File::getSpecialLocation(File::userHomeDirectory));

//------------------------------------------------------------------------------
FilePlayerControl::FilePlayerControl(FilePlayerProcessor* proc)
    : processor(proc),
      playing(false)
{
    fileDisplay = std::make_unique<WaveformDisplay>();
    addAndMakeVisible(*fileDisplay);
    fileDisplay->setName("fileDisplay");

    filename = std::make_unique<FilenameComponent>("filename",
                                                   File(),
                                                   true,
                                                   false,
                                                   false,
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

    loopButton = std::make_unique<ToggleButton>("loopButton");
    addAndMakeVisible(*loopButton);
    loopButton->setTooltip("Loop this file");
    loopButton->setButtonText("Loop");
    loopButton->addListener(this);

    playPauseButton = std::make_unique<DrawableButton>("playPauseButton", DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*playPauseButton);
    playPauseButton->setName("playPauseButton");

    rtzButton = std::make_unique<DrawableButton>("rtzButton", DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*rtzButton);
    rtzButton->setName("rtzButton");

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

    loopButton->setToggleState(processor->getParameter(FilePlayerProcessor::Looping) > 0.5f,
                               false);
    syncButton->setToggleState(processor->getParameter(FilePlayerProcessor::SyncToMainTransport) > 0.5f,
                               false);

    filename->addListener(this);
    fileDisplay->addChangeListener(this);
    processor->addChangeListener(this);

    startTimer(60);

    setSize(300, 100);
}

//------------------------------------------------------------------------------
FilePlayerControl::~FilePlayerControl()
{
    processor->removeChangeListener(this);
    removeAllChildren();
}

//------------------------------------------------------------------------------
void FilePlayerControl::paint(Graphics& /*g*/)
{
}

//------------------------------------------------------------------------------
void FilePlayerControl::resized()
{
    fileDisplay->setBounds(0, 28, getWidth() - 2, getHeight() - 48);
    filename->setBounds(0, 0, getWidth() - 58, 24);
    syncButton->setBounds(0, getHeight() - 23, 168, 24);
    loopButton->setBounds(176, getHeight() - 23, 56, 24);
    playPauseButton->setBounds(getWidth() - 54, 0, 24, 24);
    rtzButton->setBounds(getWidth() - 26, 0, 24, 24);
}

//------------------------------------------------------------------------------
void FilePlayerControl::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == syncButton.get())
    {
        bool val = syncButton->getToggleState();
        processor->setParameter(FilePlayerProcessor::SyncToMainTransport,
                                val ? 1.0f : 0.0f);
    }
    else if (buttonThatWasClicked == loopButton.get())
    {
        bool val = loopButton->getToggleState();
        processor->setParameter(FilePlayerProcessor::Looping,
                                val ? 1.0f : 0.0f);
    }
    else if (buttonThatWasClicked == playPauseButton.get())
    {
        if (!playing)
            playPauseButton->setImages(pauseImage.get());
        else
            playPauseButton->setImages(playImage.get());
        playing = !playing;

        processor->setParameter(FilePlayerProcessor::Play, 1.0f);
    }
    else if (buttonThatWasClicked == rtzButton.get())
    {
        processor->setParameter(FilePlayerProcessor::ReturnToZero, 1.0f);
        fileDisplay->setReadPointer(0.0f);
    }
}

//------------------------------------------------------------------------------
void FilePlayerControl::filenameComponentChanged(FilenameComponent* filenameComp)
{
    File phil = filenameComp->getCurrentFile();
    processor->setFile(phil);
    fileDisplay->setFile(phil);
    lastDir = phil.getParentDirectory();
}

//------------------------------------------------------------------------------
void FilePlayerControl::timerCallback()
{
    if (playing)
        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
}

//------------------------------------------------------------------------------
void FilePlayerControl::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == fileDisplay.get())
    {
        processor->setParameter(FilePlayerProcessor::ReadPosition,
                                fileDisplay->getReadPointer());
        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
    }
    else if (source == processor)
    {
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

        fileDisplay->setReadPointer(static_cast<float>(processor->getReadPosition()));
        loopButton->setToggleState(processor->getParameter(FilePlayerProcessor::Looping) > 0.5f,
                                   false);
        syncButton->setToggleState(processor->getParameter(FilePlayerProcessor::SyncToMainTransport) > 0.5f,
                                   false);
    }
}

//------------------------------------------------------------------------------
void FilePlayerControl::setWaveformBackground(const Colour& col)
{
    fileDisplay->setBackgroundColour(col);
}
