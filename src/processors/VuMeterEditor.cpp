// VuMeterEditor.cpp - The control and editor for VuMeterProcessor.
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

#include "VuMeterEditor.h"

#include "ColourScheme.h"
#include "VuMeterProcessor.h"

#include <cmath>

VuMeterControl::VuMeterControl(VuMeterProcessor* proc) : processor(proc), levelLeft(0.0f), levelRight(0.0f) {
    startTimer(60);

    setSize(64, 128);
}

VuMeterControl::~VuMeterControl() {}

void VuMeterControl::paint(Graphics& g) {
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

    ColourGradient grad1(topColour1, 0.0f, 0.0f, bottomColour, 0.0f, heightLeft, false);
    ColourGradient grad2(topColour2, 0.0f, 0.0f, bottomColour, 0.0f, heightRight, false);

    if (levelLeft >= 0.0f) {
        g.setColour(colours["VU Meter Over Colour"]);
        g.fillRect(0.0f, 0.0f, (width * 0.5f) - 2.0f, redSize);
    }

    if (levelLeft > -60.0f) {
        g.setGradientFill(grad1);
        g.fillRect(0.0f, height - heightLeft - 4.0f, (width * 0.5f) - 2.0f, heightLeft);
    }

    if (levelRight >= 0.0f) {
        g.setColour(colours["VU Meter Over Colour"]);
        g.fillRect((width * 0.5f) + 2.0f, 0.0f, (width * 0.5f) - 2.0f, redSize);
    }

    if (levelRight > -60.0f) {
        g.setGradientFill(grad2);
        g.fillRect((width * 0.5f) + 2.0f, height - heightRight - 4.0f, (width * 0.5f) - 2.0f, heightRight);
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

    g.drawText("0dB", static_cast<int>((width / 2.0f) - textSize), static_cast<int>(redSize - textSize),
               static_cast<int>(textSize * 2.0f), static_cast<int>(textSize * 2.0f), Justification::centred, false);
    g.drawText("6dB", static_cast<int>((width / 2.0f) - textSize), static_cast<int>(sixDb - textSize),
               static_cast<int>(textSize * 2.0f), static_cast<int>(textSize * 2.0f), Justification::centred, false);
    g.drawText("12dB", static_cast<int>((width / 2.0f) - (textSize * 2.0f)), static_cast<int>(twelveDb - textSize),
               static_cast<int>(textSize * 4.0f), static_cast<int>(textSize * 2.0f), Justification::centred, false);
    g.drawText("24dB", static_cast<int>((width / 2.0f) - (textSize * 2.0f)), static_cast<int>(twentyFourDb - textSize),
               static_cast<int>(textSize * 4.0f), static_cast<int>(textSize * 2.0f), Justification::centred, false);
    g.drawText("48dB", static_cast<int>((width / 2.0f) - (textSize * 2.0f)), static_cast<int>(fortyEightDb - textSize),
               static_cast<int>(textSize * 4.0f), static_cast<int>(textSize * 2.0f), Justification::centred, false);
}

void VuMeterControl::resized() {}

void VuMeterControl::timerCallback() {
    if (processor) {
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

VuMeterEditor::VuMeterEditor(AudioProcessor* processor, const Rectangle<int>& windowBounds)
    : AudioProcessorEditor(processor), parentBounds(windowBounds), setPos(false) {
    meter = std::make_unique<VuMeterControl>(dynamic_cast<VuMeterProcessor*>(processor));
    addAndMakeVisible(*meter);

    setSize(128, 256);
}

VuMeterEditor::~VuMeterEditor() {
    if (auto* proc = dynamic_cast<VuMeterProcessor*>(getAudioProcessor())) {
        if (getParentComponent())
            parentBounds = getTopLevelComponent()->getBounds();

        proc->updateEditorBounds(parentBounds);
    }

    removeAllChildren();
    getAudioProcessor()->editorBeingDeleted(this);
}

void VuMeterEditor::resized() {
    meter->setSize(getWidth(), getHeight());
}

void VuMeterEditor::paint(Graphics& g) {
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}
