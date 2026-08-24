// MetronomeControl.cpp - UI control for the metronome processor.
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

#include "MetronomeControl.h"
#include "PedalboardProcessors.h"
#include "JuceHelperStuff.h"
#include "Vectors.h"
#include "ColourScheme.h"

MetronomeControl::MetronomeControl(MetronomeProcessor* proc, bool editors)
    : processor(proc),
      playing(false),
      showFileEditors(editors)
{
    syncButton = std::make_unique<ToggleButton>("syncButton");
    addAndMakeVisible(*syncButton);
    syncButton->setTooltip("Sync metronome playback to the main transport");
    syncButton->setButtonText("Sync to main transport");
    syncButton->addListener(this);

    playPauseButton = std::make_unique<DrawableButton>("playPauseButton", DrawableButton::ImageOnButtonBackground);
    addAndMakeVisible(*playPauseButton);
    playPauseButton->setName("playPauseButton");

    accentFile = std::make_unique<FilenameComponent>("accentFile",
                                                     File(),
                                                     true,
                                                     false,
                                                     false,
                                                     "*.wav;*.aif",
                                                     "",
                                                     "<no file loaded>");
    addAndMakeVisible(*accentFile);
    accentFile->setName("accentFile");

    accentLabel = std::make_unique<Label>("accentLabel", "Accent:");
    addAndMakeVisible(*accentLabel);
    accentLabel->setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::plain)));
    accentLabel->setJustificationType(Justification::centredLeft);
    accentLabel->setEditable(false, false, false);
    accentLabel->setColour(TextEditor::textColourId, Colours::black);
    accentLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    clickFile = std::make_unique<FilenameComponent>("clickFile",
                                                    File(),
                                                    true,
                                                    false,
                                                    false,
                                                    "*.wav;*.aif",
                                                    "",
                                                    "<no file loaded>");
    addAndMakeVisible(*clickFile);
    clickFile->setName("clickFile");

    clickLabel = std::make_unique<Label>("clickLabel", "Click:");
    addAndMakeVisible(*clickLabel);
    clickLabel->setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::plain)));
    clickLabel->setJustificationType(Justification::centredLeft);
    clickLabel->setEditable(false, false, false);
    clickLabel->setColour(TextEditor::textColourId, Colours::black);
    clickLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    numeratorLabel = std::make_unique<Label>("numeratorLabel", "4");
    addAndMakeVisible(*numeratorLabel);
    numeratorLabel->setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultSerifFontName(), 250.0f, juce::Font::bold)));
    numeratorLabel->setJustificationType(Justification::centred);
    numeratorLabel->setEditable(true, true, false);
    numeratorLabel->setColour(TextEditor::textColourId, Colours::black);
    numeratorLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));
    numeratorLabel->addListener(this);

    denominatorLabel = std::make_unique<Label>("denominatorLabel", "4");
    addAndMakeVisible(*denominatorLabel);
    denominatorLabel->setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultSerifFontName(), 250.0f, juce::Font::bold)));
    denominatorLabel->setJustificationType(Justification::centred);
    denominatorLabel->setEditable(true, true, false);
    denominatorLabel->setColour(TextEditor::textColourId, Colours::black);
    denominatorLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));
    denominatorLabel->addListener(this);

    separatorLabel = std::make_unique<Label>("separatorLabel", "/");
    addAndMakeVisible(*separatorLabel);
    separatorLabel->setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultSerifFontName(), 250.0f, juce::Font::bold)));
    separatorLabel->setJustificationType(Justification::centred);
    separatorLabel->setEditable(false, false, false);
    separatorLabel->setColour(TextEditor::textColourId, Colours::black);
    separatorLabel->setColour(TextEditor::backgroundColourId, Colour(0x0));

    String tempstr;

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
    playPauseButton->setTooltip("Play/pause metronome");

    clickFile->addListener(this);
    accentFile->addListener(this);
    processor->addChangeListener(this);

    tempstr << static_cast<int>(processor->getParameter(MetronomeProcessor::Numerator));
    numeratorLabel->setText(tempstr, dontSendNotification);
    tempstr = "";
    tempstr << static_cast<int>(processor->getParameter(MetronomeProcessor::Denominator));
    denominatorLabel->setText(tempstr, dontSendNotification);

    clickFile->setCurrentFile(processor->getClickFile(), false);
    accentFile->setCurrentFile(processor->getAccentFile(), false);

    if (!showFileEditors)
    {
        clickLabel->setVisible(false);
        clickFile->setVisible(false);
        accentLabel->setVisible(false);
        accentFile->setVisible(false);
    }

    setSize(170, 100);
}

MetronomeControl::~MetronomeControl()
{
    processor->removeChangeListener(this);
    removeAllChildren();
}

void MetronomeControl::paint(Graphics& /*g*/)
{
}

void MetronomeControl::resized()
{
    syncButton->setBounds(0, getHeight() - 23, 168, 24);
    playPauseButton->setBounds(getWidth() - 26, 0, 24, 24);
    accentFile->setBounds(56, 24, getWidth() - 58, 24);
    accentLabel->setBounds(0, 24, 64, 24);
    clickFile->setBounds(56, 0, getWidth() - 84, 24);
    clickLabel->setBounds(0, 0, 48, 24);
    numeratorLabel->setBounds(0, 48, proportionOfWidth(0.4896f), getHeight() - 71);
    denominatorLabel->setBounds(proportionOfWidth(0.5104f), 48, proportionOfWidth(0.4896f), getHeight() - 71);
    separatorLabel->setBounds(proportionOfWidth(0.4301f), 48, proportionOfWidth(0.1399f), getHeight() - 71);

    float fontSize = getHeight() * (250.0f / 400.0f);
    juce::Font resizedFont(juce::FontOptions(juce::Font::getDefaultSerifFontName(),
                                              (fontSize > 14.0f) ? fontSize : 14.0f,
                                              juce::Font::bold));

    numeratorLabel->setFont(resizedFont);
    denominatorLabel->setFont(resizedFont);
    separatorLabel->setFont(resizedFont);

    if (!showFileEditors)
    {
        playPauseButton->setTopLeftPosition(0, 0);
        numeratorLabel->setBounds(0, 24, proportionOfWidth(0.4896f), getHeight() - 48);
        denominatorLabel->setBounds(proportionOfWidth(0.5104f), 24, proportionOfWidth(0.4896f), getHeight() - 48);
        separatorLabel->setBounds(proportionOfWidth(0.4301f), 24, proportionOfWidth(0.1399f), getHeight() - 48);
    }

    String tempstr;
    tempstr << static_cast<int>(processor->getParameter(MetronomeProcessor::Numerator));
    numeratorLabel->setText(tempstr, dontSendNotification);
    tempstr = "";
    tempstr << static_cast<int>(processor->getParameter(MetronomeProcessor::Denominator));
    denominatorLabel->setText(tempstr, dontSendNotification);
    syncButton->setToggleState(processor->getParameter(MetronomeProcessor::SyncToMainTransport) > 0.5f, false);
}

void MetronomeControl::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == syncButton.get())
    {
        bool val = syncButton->getToggleState();
        processor->setParameter(MetronomeProcessor::SyncToMainTransport,
                                val ? 1.0f : 0.0f);
    }
    else if (buttonThatWasClicked == playPauseButton.get())
    {
        if (!playing)
            playPauseButton->setImages(pauseImage.get());
        else
            playPauseButton->setImages(playImage.get());
        playing = !playing;
        processor->setParameter(MetronomeProcessor::Play, 1.0f);
    }
}

void MetronomeControl::labelTextChanged(Label* labelThatHasChanged)
{
    if (labelThatHasChanged == numeratorLabel.get())
    {
        float tempf = static_cast<float>(numeratorLabel->getText().getIntValue());
        processor->setParameter(MetronomeProcessor::Numerator, tempf);
    }
    else if (labelThatHasChanged == denominatorLabel.get())
    {
        float tempf = static_cast<float>(denominatorLabel->getText().getIntValue());
        processor->setParameter(MetronomeProcessor::Denominator, tempf);
    }
}

void MetronomeControl::filenameComponentChanged(FilenameComponent* fileComponentThatHasChanged)
{
    if (fileComponentThatHasChanged == accentFile.get())
        processor->setAccentFile(fileComponentThatHasChanged->getCurrentFile());
    else if (fileComponentThatHasChanged == clickFile.get())
        processor->setClickFile(fileComponentThatHasChanged->getCurrentFile());
}

void MetronomeControl::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == processor)
    {
        String tempstr;

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

        tempstr << static_cast<int>(processor->getParameter(MetronomeProcessor::Numerator));
        numeratorLabel->setText(tempstr, dontSendNotification);
        tempstr = "";
        tempstr << static_cast<int>(processor->getParameter(MetronomeProcessor::Denominator));
        denominatorLabel->setText(tempstr, dontSendNotification);
        syncButton->setToggleState(processor->getParameter(MetronomeProcessor::SyncToMainTransport) > 0.5f, false);
    }
}
