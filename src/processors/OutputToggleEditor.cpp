// OutputToggleEditor.cpp - The control and editor for OutputToggleProcessor.
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

#include "OutputToggleEditor.h"
#include "OutputToggleProcessor.h"
#include "JuceHelperStuff.h"
#include "ColourScheme.h"
#include "Vectors.h"

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

OutputToggleControl::~OutputToggleControl()
{
    removeAllChildren();
}

void OutputToggleControl::timerCallback()
{
    toggleButton->setToggleState(processor->getParameter(0) > 0.5f, false);
}

void OutputToggleControl::buttonClicked(Button* /*button*/)
{
    processor->setParameter(0, toggleButton->getToggleState() ? 1.0f : 0.0f);
}

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

void OutputToggleEditor::resized()
{
    toggleButton->setSize(getWidth(), getHeight());
}

void OutputToggleEditor::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

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

void OutputToggleEditor::buttonClicked(Button* /*button*/)
{
    getAudioProcessor()->setParameter(0, toggleButton->getToggleState() ? 1.0f : 0.0f);
}
