// InternalFilters.cpp - Internal plugin format for built-in processors.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
//
// Derived from the JUCE audio plugin host example by Raw Material Software.
// Modified by Niall Moody for Pedalboard2, and further modified for Pedalboard3.
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

#include "InternalFilters.h"

#include "OscMappingManager.h"

InternalPluginFormat::InternalPluginFormat() {
    {
        AudioProcessorGraph::AudioGraphIOProcessor p(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
        p.fillInPluginDescription(audioOutDesc);
    }

    {
        AudioProcessorGraph::AudioGraphIOProcessor p(AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
        p.fillInPluginDescription(audioInDesc);
    }

    {
        AudioProcessorGraph::AudioGraphIOProcessor p(AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
        p.fillInPluginDescription(midiInDesc);
    }

    {
        OscInput p;
        p.fillInPluginDescription(oscInputDesc);
    }

    // Additional processor descriptions will be added in Phase 3
    // when the built-in processors are ported.
}

void InternalPluginFormat::createPluginInstance(const PluginDescription& desc, double initialSampleRate,
                                                int initialBufferSize, PluginCreationCallback callback) {
    juce::ignoreUnused(initialSampleRate, initialBufferSize);

    std::unique_ptr<AudioPluginInstance> instance;

    if (desc.name == audioOutDesc.name)
        instance = std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor>(
            AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
    else if (desc.name == audioInDesc.name)
        instance = std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor>(
            AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
    else if (desc.name == midiInDesc.name)
        instance = std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor>(
            AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
    else if (desc.name == oscInputDesc.name)
        instance = std::make_unique<OscInput>();

    // Additional processor creation will be added in Phase 3

    const String error = (instance == nullptr) ? "Could not create internal plugin" : String();
    callback(std::move(instance), error);
}

const PluginDescription* InternalPluginFormat::getDescriptionFor(const InternalFilterType type) {
    switch (type) {
    case audioInputFilter:
        return &audioInDesc;
    case audioOutputFilter:
        return &audioOutDesc;
    case midiInputFilter:
        return &midiInDesc;
    case oscInputFilter:
        return &oscInputDesc;
    default:
        return nullptr;
    }
}

void InternalPluginFormat::getAllTypes(OwnedArray<PluginDescription>& results) {
    for (int i = 0; i < static_cast<int>(endOfFilterTypes); ++i)
        results.add(new PluginDescription(*getDescriptionFor(static_cast<InternalFilterType>(i))));
}
