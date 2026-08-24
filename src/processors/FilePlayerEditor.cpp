// FilePlayerEditor.cpp - The full editor for FilePlayerProcessor.
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

#include "FilePlayerEditor.h"
#include "FilePlayerProcessor.h"
#include "ColourScheme.h"

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

void FilePlayerEditor::resized()
{
    controls->setSize(getWidth() - 8, getHeight() - 8);
}

void FilePlayerEditor::paint(Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Window Background"]);
}

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
