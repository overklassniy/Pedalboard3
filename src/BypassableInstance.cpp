// BypassableInstance.cpp - Wrapper providing bypass for AudioPluginInstance.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
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

#include "BypassableInstance.h"

//------------------------------------------------------------------------------
BypassableInstance::BypassableInstance(std::unique_ptr<AudioPluginInstance> plug)
    : plugin(std::move(plug))
    , tempBuffer(2, 4096)
    , bypassRamp(0.0f)
{
    jassert(plugin);

    setPlayConfigDetails(plugin->getTotalNumInputChannels(),
                         plugin->getTotalNumOutputChannels(),
                         plugin->getSampleRate(),
                         plugin->getBlockSize());
}

//------------------------------------------------------------------------------
BypassableInstance::~BypassableInstance() = default;

//------------------------------------------------------------------------------
void BypassableInstance::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
    int numChannels = juce::jmax(plugin->getTotalNumInputChannels(),
                                 plugin->getTotalNumOutputChannels());

    jassert(numChannels > 0);

    midiCollector.reset(sampleRate);

    // Multiply by 2 to ensure we don't run out of space since we only get an estimate.
    tempBuffer.setSize(numChannels, estimatedSamplesPerBlock * 2);

    plugin->setPlayHead(getPlayHead());
    plugin->setPlayConfigDetails(plugin->getTotalNumInputChannels(),
                                 plugin->getTotalNumOutputChannels(),
                                 sampleRate,
                                 estimatedSamplesPerBlock);
    plugin->prepareToPlay(sampleRate, estimatedSamplesPerBlock);
}

//------------------------------------------------------------------------------
void BypassableInstance::releaseResources()
{
    plugin->releaseResources();
}

//------------------------------------------------------------------------------
void BypassableInstance::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    float rampVal = bypassRamp;
    juce::MidiBuffer tempMidi;

    jassert(buffer.getNumChannels() <= tempBuffer.getNumChannels());

    // Pass on any MIDI messages received via OSC.
    midiCollector.removeNextBlockOfMessages(tempMidi, buffer.getNumSamples());
    if (!midiMessages.isEmpty())
    {
        // JUCE 8: use range-for with MidiMessageMetadata instead of deprecated Iterator
        for (const auto metadata : midiMessages)
        {
            int chan = midiChannel.load();
            if (chan == 0 || metadata.getMessage().getChannel() == chan)
                tempMidi.addEvent(metadata.getMessage(), metadata.samplePosition);
        }
    }

    tempBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);

    // Fill out our temporary buffer with the original audio.
    for (int i = 0; i < buffer.getNumChannels(); ++i)
        tempBuffer.copyFrom(i, 0, buffer, i, 0, buffer.getNumSamples());

    // Get the plugin's audio.
    plugin->processBlock(buffer, tempMidi);

    // Add any new midi data back to midiMessages.
    if (!tempMidi.isEmpty())
        midiMessages.swapWith(tempMidi);

    // Mix the correct (bypassed or un-bypassed) audio back to the buffer.
    for (int j = 0; j < buffer.getNumChannels(); ++j)
    {
        auto* origData = tempBuffer.getWritePointer(j);
        auto* newData = buffer.getWritePointer(j);

        rampVal = bypassRamp;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            newData[i] = (origData[i] * rampVal) + (newData[i] * (1.0f - rampVal));

            if (bypass.load() && rampVal < 1.0f)
            {
                rampVal += 0.001f;
                if (rampVal > 1.0f)
                    rampVal = 1.0f;
            }
            else if (!bypass.load() && rampVal > 0.0f)
            {
                rampVal -= 0.001f;
                if (rampVal < 0.0f)
                    rampVal = 0.0f;
            }
        }
    }
    bypassRamp = rampVal;
}

//------------------------------------------------------------------------------
void BypassableInstance::setBypass(bool val)
{
    bypass = val;
}

//------------------------------------------------------------------------------
void BypassableInstance::setMIDIChannel(int val)
{
    midiChannel = val;
}

//------------------------------------------------------------------------------
void BypassableInstance::addMidiMessage(const juce::MidiMessage& message)
{
    int chan = midiChannel.load();
    if (chan == 0 || message.getChannel() == chan)
        midiCollector.addMessageToQueue(message);
}
