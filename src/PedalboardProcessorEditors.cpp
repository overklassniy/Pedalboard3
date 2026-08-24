// PedalboardProcessorEditors.cpp - The various editors for the app's internal processors.
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

#include "PedalboardProcessorEditors.h"
#include "PedalboardProcessors.h"
#include "JuceHelperStuff.h"
#include "ColourScheme.h"
#include "Vectors.h"

#include <cmath>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
LevelControl::LevelControl(LevelProcessor* proc)
    : processor(proc)
{
    slider = std::make_unique<Slider>();
    addAndMakeVisible(*slider);

    slider->setSliderStyle(Slider::RotaryVerticalDrag);
    slider->setTextBoxStyle(Slider::NoTextBox, true, 64, 20);
    slider->setRange(0.0, 2.0);
    slider->setValue(processor->getParameter(0) * 2.0f);
    slider->setDoubleClickReturnValue(true, 1.0);
    slider->addListener(this);
    slider->setTopLeftPosition(0, 0);
    slider->setSize(64, 64);
    slider->setColour(Slider::rotarySliderFillColourId,
                      ColourScheme::getInstance().colours["Level Dial Colour"]);

    startTimer(60);

    setSize(64, 64);
}

//------------------------------------------------------------------------------
LevelControl::~LevelControl()
{
    removeAllChildren();
}

//------------------------------------------------------------------------------
void LevelControl::timerCallback()
{
    slider->setValue(processor->getParameter(0) * 2.0f);
}

//------------------------------------------------------------------------------
void LevelControl::sliderValueChanged(Slider* /*slider*/)
{
    processor->setParameter(0, static_cast<float>(slider->getValue()) * 0.5f);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
LevelEditor::LevelEditor(AudioProcessor* processor,
                         const Rectangle<int>& windowBounds)
    : AudioProcessorEditor(processor),
      parentBounds(windowBounds),
      setPos(false)
{
    slider = std::make_unique<Slider>();
    addAndMakeVisible(*slider);

    slider->setSliderStyle(Slider::RotaryVerticalDrag);
    slider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    slider->setRange(0.0, 2.0);
    slider->setValue(processor->getParameter(0) * 2.0f);
    slider->setDoubleClickReturnValue(true, 1.0);
    slider->addListener(this);
    slider->setTopLeftPosition(0, 0);
    slider->setSize(192, 192);
    slider->setColour(Slider::rotarySliderFillColourId,
                      ColourScheme::getInstance().colours["Level Dial Colour"]);

    setSize(192, 192);
    startTimer(60);
}

//------------------------------------------------------------------------------
LevelEditor::~LevelEditor()
{
    if (auto* proc = dynamic_cast<LevelProcessor*>(getAudioProcessor()))
    {
        if (getParentComponent())
            parentBounds = getTopLevelComponent()->getBounds();

        proc->updateEditorBounds(parentBounds);
    }

    removeAllChildren();
    getAudioProcessor()->editorBeingDeleted(this);
}

//------------------------------------------------------------------------------
void LevelEditor::resized()
{
    const int h = getHeight();
    const int deskH = static_cast<int>(static_cast<float>(getParentMonitorArea().getHeight()) / 1.5f);

    slider->setSize(getWidth(), h);

    if (h > 250)
    {
        if (h < deskH)
            slider->setMouseDragSensitivity(h);
        else
            slider->setMouseDragSensitivity(deskH);
    }
    else
    {
        slider->setMouseDragSensitivity(250);
    }
}

//------------------------------------------------------------------------------
void LevelEditor::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

//------------------------------------------------------------------------------
void LevelEditor::timerCallback()
{
    slider->setValue(getAudioProcessor()->getParameter(0) * 2.0f);

    if (!setPos)
    {
        if (parentBounds.isEmpty())
        {
            setPos = true;
        }
        else if (ComponentPeer* peer = getPeer())
        {
            peer->setBounds(parentBounds, false);
            setPos = true;
        }
    }
}

//------------------------------------------------------------------------------
void LevelEditor::sliderValueChanged(Slider* /*slider*/)
{
    getAudioProcessor()->setParameter(0, static_cast<float>(slider->getValue()) * 0.5f);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
FilePlayerEditor::FilePlayerEditor(FilePlayerProcessor* processor,
                                   const Rectangle<int>& windowBounds)
    : AudioProcessorEditor(processor),
      parentBounds(windowBounds),
      setPos(false)
{
    controls = std::make_unique<FilePlayerControl>(processor);
    controls->setWaveformBackground(Colour(0xFFEEECE1).darker(0.05f));
    controls->setTopLeftPosition(4, 4);
    controls->setSize(getWidth() - 8, getHeight() - 8);
    addAndMakeVisible(*controls);

    setSize(600, 200);
    startTimer(60);
}

//------------------------------------------------------------------------------
FilePlayerEditor::~FilePlayerEditor()
{
    if (auto* proc = dynamic_cast<FilePlayerProcessor*>(getAudioProcessor()))
    {
        if (getParentComponent())
            parentBounds = getTopLevelComponent()->getBounds();

        proc->updateEditorBounds(parentBounds);
    }

    removeAllChildren();
    getAudioProcessor()->editorBeingDeleted(this);
}

//------------------------------------------------------------------------------
void FilePlayerEditor::resized()
{
    controls->setSize(getWidth() - 8, getHeight() - 8);
}

//------------------------------------------------------------------------------
void FilePlayerEditor::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

//------------------------------------------------------------------------------
void FilePlayerEditor::timerCallback()
{
    if (!setPos)
    {
        if (parentBounds.isEmpty())
        {
            setPos = true;
        }
        else if (ComponentPeer* peer = getPeer())
        {
            peer->setBounds(parentBounds, false);
            setPos = true;
            stopTimer();
        }
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
OutputToggleControl::OutputToggleControl(OutputToggleProcessor* proc)
    : processor(proc)
{
    std::unique_ptr<Drawable> im1(JuceHelperStuff::loadSVGFromMemory(Vectors::outputtoggle1_svg,
                                                                      Vectors::outputtoggle1_svgSize));
    std::unique_ptr<Drawable> im2(JuceHelperStuff::loadSVGFromMemory(Vectors::outputtoggle2_svg,
                                                                      Vectors::outputtoggle2_svgSize));

    toggleButton = std::make_unique<DrawableButton>("toggleButton",
                                                     DrawableButton::ImageFitted);
    toggleButton->setImages(im1.get(), nullptr, nullptr, nullptr, im2.get());
    toggleButton->setColour(DrawableButton::backgroundColourId,
                            Colours::transparentBlack);
    toggleButton->setColour(DrawableButton::backgroundOnColourId,
                            Colours::transparentBlack);
    toggleButton->setClickingTogglesState(true);
    toggleButton->setTopLeftPosition(0, 0);
    toggleButton->setSize(48, 48);
    toggleButton->addListener(this);
    addAndMakeVisible(*toggleButton);

    startTimer(60);

    setSize(48, 48);
}

//------------------------------------------------------------------------------
OutputToggleControl::~OutputToggleControl()
{
    removeAllChildren();
}

//------------------------------------------------------------------------------
void OutputToggleControl::timerCallback()
{
    toggleButton->setToggleState(processor->getParameter(0) > 0.5f, false);
}

//------------------------------------------------------------------------------
void OutputToggleControl::buttonClicked(Button* /*button*/)
{
    processor->setParameter(0, toggleButton->getToggleState() ? 1.0f : 0.0f);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
OutputToggleEditor::OutputToggleEditor(AudioProcessor* processor,
                                       const Rectangle<int>& windowBounds)
    : AudioProcessorEditor(processor),
      parentBounds(windowBounds),
      setPos(false)
{
    std::unique_ptr<Drawable> im1(JuceHelperStuff::loadSVGFromMemory(Vectors::outputtoggle1_svg,
                                                                      Vectors::outputtoggle1_svgSize));
    std::unique_ptr<Drawable> im2(JuceHelperStuff::loadSVGFromMemory(Vectors::outputtoggle2_svg,
                                                                      Vectors::outputtoggle2_svgSize));

    toggleButton = std::make_unique<DrawableButton>("toggleButton",
                                                     DrawableButton::ImageFitted);
    toggleButton->setImages(im1.get(), nullptr, nullptr, nullptr, im2.get());
    toggleButton->setColour(DrawableButton::backgroundColourId,
                            Colours::transparentBlack);
    toggleButton->setColour(DrawableButton::backgroundOnColourId,
                            Colours::transparentBlack);
    toggleButton->setClickingTogglesState(true);
    toggleButton->setTopLeftPosition(0, 0);
    toggleButton->setSize(48, 48);
    toggleButton->addListener(this);
    addAndMakeVisible(*toggleButton);

    setSize(192, 192);
    startTimer(60);
}

//------------------------------------------------------------------------------
OutputToggleEditor::~OutputToggleEditor()
{
    if (auto* proc = dynamic_cast<OutputToggleProcessor*>(getAudioProcessor()))
    {
        if (getParentComponent())
            parentBounds = getTopLevelComponent()->getBounds();

        proc->updateEditorBounds(parentBounds);
    }

    removeAllChildren();
    getAudioProcessor()->editorBeingDeleted(this);
}

//------------------------------------------------------------------------------
void OutputToggleEditor::resized()
{
    toggleButton->setSize(getWidth(), getHeight());
}

//------------------------------------------------------------------------------
void OutputToggleEditor::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

//------------------------------------------------------------------------------
void OutputToggleEditor::timerCallback()
{
    toggleButton->setToggleState(getAudioProcessor()->getParameter(0) > 0.5f, false);

    if (!setPos)
    {
        if (parentBounds.isEmpty())
        {
            setPos = true;
        }
        else if (ComponentPeer* peer = getPeer())
        {
            peer->setBounds(parentBounds, false);
            setPos = true;
        }
    }
}

//------------------------------------------------------------------------------
void OutputToggleEditor::buttonClicked(Button* /*button*/)
{
    getAudioProcessor()->setParameter(0, toggleButton->getToggleState() ? 1.0f : 0.0f);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
VuMeterControl::VuMeterControl(VuMeterProcessor* proc)
    : processor(proc),
      levelLeft(0.0f),
      levelRight(0.0f)
{
    startTimer(60);

    setSize(64, 128);
}

//------------------------------------------------------------------------------
VuMeterControl::~VuMeterControl()
{
}

//------------------------------------------------------------------------------
void VuMeterControl::paint(Graphics& g)
{
    float textSize;
    float levLeft = (levelLeft < 0.0f) ? (levelLeft / 60.0f) + 1.0f : 1.0f;
    float levRight = (levelRight < 0.0f) ? (levelRight / 60.0f) + 1.0f : 1.0f;
    float width = static_cast<float>(getWidth());
    float height = static_cast<float>(getHeight());
    float redSize = (height < 128.0f) ? 10.0f : (height * (10.0f / 128.0f));
    float heightLeft = (height - redSize - 4.0f) * levLeft;
    float heightRight = (height - redSize - 4.0f) * levRight;
    const float sixDb = redSize + ((6.0f / 60.0f) * (height - redSize - 4.0f));
    const float twelveDb = redSize + ((12.0f / 60.0f) * (height - redSize - 4.0f));
    const float twentyFourDb = redSize + ((24.0f / 60.0f) * (height - redSize - 4.0f));
    const float fortyEightDb = redSize + ((48.0f / 60.0f) * (height - redSize - 4.0f));
    std::map<String, Colour>& colours = ColourScheme::getInstance().colours;

    Colour topColour1 = colours["VU Meter Upper Colour"].withMultipliedBrightness(levLeft);
    Colour topColour2 = colours["VU Meter Upper Colour"].withMultipliedBrightness(levRight);
    Colour bottomColour = colours["VU Meter Lower Colour"];

    ColourGradient grad1(topColour1,
                           0.0f,
                           0.0f,
                           bottomColour,
                           0.0f,
                           heightLeft,
                           false);
    ColourGradient grad2(topColour2,
                           0.0f,
                           0.0f,
                           bottomColour,
                           0.0f,
                           heightRight,
                           false);

    if (levelLeft >= 0.0f)
    {
        g.setColour(colours["VU Meter Over Colour"]);
        g.fillRect(0.0f, 0.0f, (width * 0.5f) - 2.0f, redSize);
    }

    if (levelLeft > -60.0f)
    {
        g.setGradientFill(grad1);
        g.fillRect(0.0f,
                   height - heightLeft - 4.0f,
                   (width * 0.5f) - 2.0f,
                   heightLeft);
    }

    if (levelRight >= 0.0f)
    {
        g.setColour(colours["VU Meter Over Colour"]);
        g.fillRect((width * 0.5f) + 2.0f, 0.0f, (width * 0.5f) - 2.0f, redSize);
    }

    if (levelRight > -60.0f)
    {
        g.setGradientFill(grad2);
        g.fillRect((width * 0.5f) + 2.0f,
                   height - heightRight - 4.0f,
                   (width * 0.5f) - 2.0f,
                   heightRight);
    }

    g.setColour(colours["Text Colour"].withAlpha(0.25f));
    g.drawLine(0.0f, redSize, width, redSize);
    g.drawLine(0.0f, sixDb, width, sixDb);
    g.drawLine(0.0f, twelveDb, width, twelveDb);
    g.drawLine(0.0f, twentyFourDb, width, twentyFourDb);
    g.drawLine(0.0f, fortyEightDb, width, fortyEightDb);

    if (width <= 64.0f)
        textSize = 8.0f;
    else if (width > 192.0f)
        textSize = 24.0f;
    else
        textSize = width / 8.0f;

    g.setColour(colours["Text Colour"].withAlpha(0.5f));
    g.setFont(textSize);

    g.drawText("0dB",
               static_cast<int>((width / 2.0f) - textSize),
               static_cast<int>(redSize - textSize),
               static_cast<int>(textSize * 2.0f),
               static_cast<int>(textSize * 2.0f),
               Justification::centred,
               false);
    g.drawText("6dB",
               static_cast<int>((width / 2.0f) - textSize),
               static_cast<int>(sixDb - textSize),
               static_cast<int>(textSize * 2.0f),
               static_cast<int>(textSize * 2.0f),
               Justification::centred,
               false);
    g.drawText("12dB",
               static_cast<int>((width / 2.0f) - (textSize * 2.0f)),
               static_cast<int>(twelveDb - textSize),
               static_cast<int>(textSize * 4.0f),
               static_cast<int>(textSize * 2.0f),
               Justification::centred,
               false);
    g.drawText("24dB",
               static_cast<int>((width / 2.0f) - (textSize * 2.0f)),
               static_cast<int>(twentyFourDb - textSize),
               static_cast<int>(textSize * 4.0f),
               static_cast<int>(textSize * 2.0f),
               Justification::centred,
               false);
    g.drawText("48dB",
               static_cast<int>((width / 2.0f) - (textSize * 2.0f)),
               static_cast<int>(fortyEightDb - textSize),
               static_cast<int>(textSize * 4.0f),
               static_cast<int>(textSize * 2.0f),
               Justification::centred,
               false);
}

//------------------------------------------------------------------------------
void VuMeterControl::resized()
{
}

//------------------------------------------------------------------------------
void VuMeterControl::timerCallback()
{
    if (processor)
    {
        float levLeft = processor->getLeftLevel();
        float levRight = processor->getRightLevel();
        const float minus60 = static_cast<float>(std::pow(10.0, -60.0 / 20.0));

        if (levLeft > minus60)
            levelLeft = 20.0f * static_cast<float>(std::log10(levLeft));
        else
            levelLeft = -60.0f;

        if (levRight > minus60)
            levelRight = 20.0f * static_cast<float>(std::log10(levRight));
        else
            levelRight = -60.0f;

        repaint();
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
VuMeterEditor::VuMeterEditor(AudioProcessor* processor,
                             const Rectangle<int>& windowBounds)
    : AudioProcessorEditor(processor),
      parentBounds(windowBounds),
      setPos(false)
{
    meter = std::make_unique<VuMeterControl>(dynamic_cast<VuMeterProcessor*>(processor));
    addAndMakeVisible(*meter);

    setSize(128, 256);
}

//------------------------------------------------------------------------------
VuMeterEditor::~VuMeterEditor()
{
    if (auto* proc = dynamic_cast<VuMeterProcessor*>(getAudioProcessor()))
    {
        if (getParentComponent())
            parentBounds = getTopLevelComponent()->getBounds();

        proc->updateEditorBounds(parentBounds);
    }

    removeAllChildren();
    getAudioProcessor()->editorBeingDeleted(this);
}

//------------------------------------------------------------------------------
void VuMeterEditor::resized()
{
    meter->setSize(getWidth(), getHeight());
}

//------------------------------------------------------------------------------
void VuMeterEditor::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
AudioRecorderEditor::AudioRecorderEditor(RecorderProcessor* processor,
                                         const Rectangle<int>& windowBounds,
                                         AudioThumbnail& thumbnail)
    : AudioProcessorEditor(processor),
      parentBounds(windowBounds),
      setPos(false)
{
    controls = std::make_unique<AudioRecorderControl>(processor, thumbnail);
    controls->setWaveformBackground(Colour(0xFFEEECE1).darker(0.05f));
    controls->setTopLeftPosition(4, 4);
    controls->setSize(getWidth() - 8, getHeight() - 8);
    addAndMakeVisible(*controls);

    setSize(600, 200);
    startTimer(60);
}

//------------------------------------------------------------------------------
AudioRecorderEditor::~AudioRecorderEditor()
{
    if (auto* proc = dynamic_cast<RecorderProcessor*>(getAudioProcessor()))
    {
        if (getParentComponent())
            parentBounds = getTopLevelComponent()->getBounds();

        proc->updateEditorBounds(parentBounds);
    }

    removeAllChildren();
    getAudioProcessor()->editorBeingDeleted(this);
}

//------------------------------------------------------------------------------
void AudioRecorderEditor::resized()
{
    controls->setSize(getWidth() - 8, getHeight() - 8);
}

//------------------------------------------------------------------------------
void AudioRecorderEditor::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

//------------------------------------------------------------------------------
void AudioRecorderEditor::timerCallback()
{
    if (!setPos)
    {
        if (parentBounds.isEmpty())
        {
            setPos = true;
        }
        else if (ComponentPeer* peer = getPeer())
        {
            peer->setBounds(parentBounds, false);
            setPos = true;
            stopTimer();
        }
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
MetronomeEditor::MetronomeEditor(MetronomeProcessor* processor,
                                 const Rectangle<int>& windowBounds)
    : AudioProcessorEditor(processor),
      parentBounds(windowBounds),
      setPos(false)
{
    controls = std::make_unique<MetronomeControl>(processor, true);
    controls->setTopLeftPosition(4, 4);
    controls->setSize(getWidth() - 8, getHeight() - 8);
    addAndMakeVisible(*controls);

    setSize(400, 200);
    startTimer(60);
}

//------------------------------------------------------------------------------
MetronomeEditor::~MetronomeEditor()
{
    if (auto* proc = dynamic_cast<MetronomeProcessor*>(getAudioProcessor()))
    {
        if (getParentComponent())
            parentBounds = getTopLevelComponent()->getBounds();

        proc->updateEditorBounds(parentBounds);
    }

    removeAllChildren();
    getAudioProcessor()->editorBeingDeleted(this);
}

//------------------------------------------------------------------------------
void MetronomeEditor::resized()
{
    controls->setSize(getWidth() - 8, getHeight() - 8);
}

//------------------------------------------------------------------------------
void MetronomeEditor::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

//------------------------------------------------------------------------------
void MetronomeEditor::timerCallback()
{
    if (!setPos)
    {
        if (parentBounds.isEmpty())
        {
            setPos = true;
        }
        else if (ComponentPeer* peer = getPeer())
        {
            peer->setBounds(parentBounds, false);
            setPos = true;
            stopTimer();
        }
    }
}
