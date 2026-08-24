// CrossfadeMixer.cpp - Glitch-free patch switching crossfade mixer.
//
// This file is part of Pedalboard3, an audio plugin host.
// Ported from the Pedalboard3-VST3 fork by Project12x.
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

#include "CrossfadeMixer.h"

CrossfadeMixerProcessor::CrossfadeMixerProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", AudioChannelSet::stereo(), true)
                         .withOutput("Output", AudioChannelSet::stereo(), true))
{
}

void CrossfadeMixerProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
}

void CrossfadeMixerProcessor::startFadeOut(int durationMs)
{
    if (durationMs <= 0)
        durationMs = defaultFadeMs;

    // Calculate samples for fade
    int fadeSamples = static_cast<int>((durationMs / 1000.0) * currentSampleRate);
    if (fadeSamples < 1)
        fadeSamples = 1;

    // Set fade parameters (negative increment for fade out)
    fadeIncrement.store(-1.0f / static_cast<float>(fadeSamples));
    fadingOut.store(true);
    fading.store(true);
}

void CrossfadeMixerProcessor::startFadeIn(int durationMs)
{
    if (durationMs <= 0)
        durationMs = defaultFadeMs;

    // Calculate samples for fade
    int fadeSamples = static_cast<int>((durationMs / 1000.0) * currentSampleRate);
    if (fadeSamples < 1)
        fadeSamples = 1;

    // Set fade parameters (positive increment for fade in)
    fadeIncrement.store(1.0f / static_cast<float>(fadeSamples));
    fadingOut.store(false);
    fading.store(true);
}

void CrossfadeMixerProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& /*midi*/)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // If not fading, apply current gain and return
    if (!fading.load())
    {
        float gain = fadeGain.load();
        if (gain < 0.999f)
        {
            // Apply constant gain (we're in silent state)
            buffer.applyGain(gain);
        }
        // gain >= 0.999f means full volume, no processing needed
        return;
    }

    // Fading - apply per-sample gain ramp
    float currentGain = fadeGain.load();
    float increment = fadeIncrement.load();
    bool isFadingOut = fadingOut.load();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Apply gain to all channels for this sample
        for (int channel = 0; channel < numChannels; ++channel)
        {
            buffer.getWritePointer(channel)[sample] *= currentGain;
        }

        // Update gain
        currentGain += increment;

        // Clamp and check for fade completion
        if (isFadingOut)
        {
            if (currentGain <= 0.0f)
            {
                currentGain = 0.0f;
                fading.store(false);
                break;
            }
        }
        else
        {
            if (currentGain >= 1.0f)
            {
                currentGain = 1.0f;
                fading.store(false);
                break;
            }
        }
    }

    // Store final gain value
    fadeGain.store(currentGain);
}
