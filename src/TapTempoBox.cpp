// TapTempoBox.cpp - Simple component letting the user tap the tempo.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
// Copyright (c) 2026 Pedalboard3 Project.
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

#include "TapTempoBox.h"
#include "PluginField.h"
#include "ColourScheme.h"

//------------------------------------------------------------------------------

TapTempoBox::TapTempoBox(PluginField* field, TextEditor* tempoEd) :
    tempo(120.0),
    pluginField(field),
    tempoEditor(tempoEd)
{
    setSize(300, 120);

    startTimer(30);
}

//------------------------------------------------------------------------------

TapTempoBox::~TapTempoBox()
{
}

//------------------------------------------------------------------------------

void TapTempoBox::paint(Graphics& g)
{
    Font smallFont(juce::FontOptions().withHeight(24.0f));
    Font bigFont(juce::FontOptions().withHeight(48.0f).withStyle("Bold"));

    g.setColour(ColourScheme::getInstance().colours["Text Colour"]);

    g.setFont(smallFont);
    g.drawText("Tap to set tempo:", 0, 0, 300, 50,
               Justification(Justification::centred), false);

    // Format the tempo to 2 decimal places.
    String tempstr = String(tempo, 2);
    tempstr << " bpm";
    g.setFont(bigFont);
    g.drawText(tempstr, 0, 50, 300, 50,
               Justification(Justification::centred), false);
}

//------------------------------------------------------------------------------

void TapTempoBox::mouseDown(const MouseEvent& e)
{
    (void) e;

    double tempTempo;
    int64 ticks = Time::getHighResolutionTicks();

    tempTempo = tapHelper.updateTempo(Time::highResolutionTicksToSeconds(ticks));
    if (tempTempo > 0.0)
    {
        tempo = tempTempo;
        pluginField->setTempo(tempo);

        tempoEditor->setText(String(tempo, 2), false);

        repaint();
    }
}

//------------------------------------------------------------------------------

void TapTempoBox::timerCallback()
{
    double newTempo = pluginField->getTempo();

    if (tempo != newTempo)
    {
        tempo = newTempo;
        repaint();
    }
}
