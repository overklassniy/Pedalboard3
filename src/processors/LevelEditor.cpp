// LevelEditor.cpp - The control and editor for LevelProcessor.
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

#include "LevelEditor.h"
#include "LevelProcessor.h"
#include "ColourScheme.h"

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

LevelControl::~LevelControl()
{
    removeAllChildren();
}

void LevelControl::timerCallback()
{
    slider->setValue(processor->getParameter(0) * 2.0f);
}

void LevelControl::sliderValueChanged(Slider* /*slider*/)
{
    processor->setParameter(0, static_cast<float>(slider->getValue()) * 0.5f);
}

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

void LevelEditor::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

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

void LevelEditor::sliderValueChanged(Slider* /*slider*/)
{
    getAudioProcessor()->setParameter(0, static_cast<float>(slider->getValue()) * 0.5f);
}
