// OscMappingManager.cpp - Dispatches OSC messages to OscMappings.
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

#include "OscMappingManager.h"
#include "BypassableInstance.h"

OscMapping::OscMapping(OscMappingManager* manager, FilterGraph* graph, uint32 pluginId, int param,
                       const String& oscAddress, int oscParam)
    : Mapping(graph, pluginId, param)
    , mappingManager(manager)
    , address(oscAddress)
    , parameter(oscParam)
{
}

OscMapping::OscMapping(OscMappingManager* manager, FilterGraph* graph, XmlElement* e)
    : Mapping(graph, e)
    , mappingManager(manager)
{
    if (e)
    {
        address = e->getStringAttribute("address");
        parameter = e->getIntAttribute("parameterIndex");
    }
}

OscMapping::~OscMapping()
{
    mappingManager->unregisterMapping(this);
}

void OscMapping::messageReceived(float val)
{
    updateParameter(val);
}

XmlElement* OscMapping::getXml() const
{
    auto* retval = new XmlElement("OscMapping");

    retval->setAttribute("pluginId", static_cast<int>(getPluginId()));
    retval->setAttribute("parameter", getParameter());
    retval->setAttribute("address", address);
    retval->setAttribute("parameterIndex", parameter);

    return retval;
}

void OscMapping::setAddress(const String& oscAddress)
{
    address = oscAddress;
    mappingManager->unregisterMapping(this);
    mappingManager->registerMapping(address, this);
}

void OscMapping::setParameterIndex(int val)
{
    parameter = val;
}

OscAppMapping::OscAppMapping(OscMappingManager* manager, const String& oscAddress, int oscParam, CommandID commandId)
    : oscManager(manager)
    , address(oscAddress)
    , parameter(oscParam)
    , id(commandId)
{
}

OscAppMapping::OscAppMapping(OscMappingManager* manager, XmlElement* e)
    : oscManager(manager)
{
    if (e)
    {
        address = e->getStringAttribute("address");
        parameter = e->getIntAttribute("parameterIndex");
        id = static_cast<CommandID>(e->getIntAttribute("commandId"));
    }
}

OscAppMapping::~OscAppMapping()
{
    oscManager->unregisterAppMapping(this);
}

XmlElement* OscAppMapping::getXml() const
{
    auto* retval = new XmlElement("OscAppMapping");

    retval->setAttribute("address", address);
    retval->setAttribute("parameterIndex", parameter);
    retval->setAttribute("commandId", static_cast<int>(id));

    return retval;
}

OscMappingManager::OscMappingManager(ApplicationCommandManager* manager)
    : appManager(manager)
{
}

OscMappingManager::~OscMappingManager()
{
    // Collect all mappings and delete them (they are owned by this manager).
    std::vector<OscMapping*> tempMappings;
    std::vector<OscAppMapping*> tempAppMappings;

    {
        const ScopedLock sl(containerLock);
        for (auto& pair : mappings)
            tempMappings.push_back(pair.second);
        for (auto& pair : appMappings)
            tempAppMappings.push_back(pair.second);
    }

    for (auto* m : tempMappings)
        delete m;
    for (auto* m : tempAppMappings)
        delete m;
}

void OscMappingManager::messageReceived(const juce::OSCMessage& message)
{
    const String address = message.getAddressPattern().toString();

    // Collect float and int arguments from the OSC message.
    // JUCE OSC arguments are accessed via OSCArgument which supports isFloat32(),
    // isInt32(), isString(), isBlob().
    std::vector<float> floatArgs;
    std::vector<int> intArgs;

    for (const auto& arg : message)
    {
        if (arg.isFloat32())
            floatArgs.push_back(arg.getFloat32());
        else if (arg.isInt32())
            intArgs.push_back(arg.getInt32());
    }

    // All container access under lock (OSC network thread vs. message thread mutations).
    const ScopedLock sl(containerLock);

    // Standard OSC mappings are all treated as floats (0->1).
    for (size_t i = 0; i < floatArgs.size(); ++i)
        handleFloatMessage(address, static_cast<int>(i), floatArgs[i]);

    // MIDI over OSC: check if any MIDI processors are registered at this address.
    auto it = midiProcessors.lower_bound(address);
    if (it != midiProcessors.end())
    {
        std::unique_ptr<juce::MidiMessage> tempMess;

        // MIDI over OSC can be delivered as:
        // 1. A blob with 4 MIDI bytes (JUCE OSC blob - we extract first 4 bytes)
        // 2. Three int arguments (status, data1, data2)
        // 3. Three float arguments (status, data1, data2) - cast to int
        if (intArgs.size() > 2)
        {
            tempMess = std::make_unique<juce::MidiMessage>(intArgs[0], intArgs[1], intArgs[2]);
        }
        else if (floatArgs.size() > 2)
        {
            tempMess = std::make_unique<juce::MidiMessage>(
                static_cast<int>(floatArgs[0]), static_cast<int>(floatArgs[1]), static_cast<int>(floatArgs[2]));
        }

        if (tempMess)
        {
            tempMess->setTimeStamp(Time::getMillisecondCounter() / 1000.0);

            for (; it != midiProcessors.upper_bound(address); ++it)
            {
                if (it->second)
                    it->second->addMidiMessage(*tempMess);
            }
        }
    }
}

void OscMappingManager::handleFloatMessage(const String& address, int index, float val)
{
    // Caller (messageReceived) already holds containerLock.

    // Check against any OscMappings.
    for (auto it = mappings.lower_bound(address); it != mappings.upper_bound(address); ++it)
    {
        if (it->second->getParameterIndex() == index)
            it->second->messageReceived(val);
    }

    // Check against any OscAppMappings (trigger on val > 0.5).
    if (val > 0.5f)
    {
        for (auto it = appMappings.lower_bound(address); it != appMappings.upper_bound(address); ++it)
        {
            if (it->second->getParameterIndex() == index)
            {
                // Application command invocation will be wired in Phase 3
                // when MainPanel is ported. For now, we just track the
                // command ID and tap tempo.
                CommandID id = it->second->getId();

                if (id != 0) // TransportTapTempo will be handled here
                {
                    // TODO: Phase 3 - invoke via MainPanel::invokeCommandFromOtherThread(id)
                    // For tap tempo:
                    // double tempo = tapHelper.updateTempo(Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks()));
                    // panel->updateTempoFromOtherThread(tempo);
                }
            }
        }
    }

    // Add to unique addresses for address learning.
    uniqueAddresses.addIfNotAlreadyThere(address);
}

void OscMappingManager::handleMIDIMessage(const String& address, const juce::MidiMessage& midiMessage)
{
    const ScopedLock sl(containerLock);

    for (auto it = midiProcessors.lower_bound(address); it != midiProcessors.upper_bound(address); ++it)
    {
        if (it->second)
            it->second->addMidiMessage(midiMessage);
    }
}

void OscMappingManager::registerMapping(const String& address, OscMapping* mapping)
{
    const ScopedLock sl(containerLock);
    jassert(mapping);
    mappings.insert({address, mapping});
}

void OscMappingManager::unregisterMapping(OscMapping* mapping)
{
    const ScopedLock sl(containerLock);
    jassert(mapping);

    for (auto it = mappings.begin(); it != mappings.end();)
    {
        if (it->second == mapping)
            it = mappings.erase(it);
        else
            ++it;
    }
}

void OscMappingManager::registerAppMapping(OscAppMapping* mapping)
{
    const ScopedLock sl(containerLock);
    jassert(mapping);
    appMappings.insert({mapping->getAddress(), mapping});
}

void OscMappingManager::unregisterAppMapping(OscAppMapping* mapping)
{
    const ScopedLock sl(containerLock);

    for (auto it = appMappings.begin(); it != appMappings.end();)
    {
        if (it->second == mapping)
            it = appMappings.erase(it);
        else
            ++it;
    }
}

void OscMappingManager::registerMIDIProcessor(const String& address, BypassableInstance* processor)
{
    const ScopedLock sl(containerLock);
    jassert(processor);

    // Remove any existing entry for this processor to avoid duplicates.
    for (auto it = midiProcessors.begin(); it != midiProcessors.end(); ++it)
    {
        if (it->second == processor)
        {
            midiProcessors.erase(it);
            break;
        }
    }

    midiProcessors.insert({address, processor});
}

void OscMappingManager::unregisterMIDIProcessor(BypassableInstance* processor)
{
    const ScopedLock sl(containerLock);

    for (auto it = midiProcessors.begin(); it != midiProcessors.end(); ++it)
    {
        if (it->second == processor)
        {
            midiProcessors.erase(it);
            break;
        }
    }
}

const String OscMappingManager::getMIDIProcessorAddress(BypassableInstance* processor) const
{
    const ScopedLock sl(containerLock);
    String retval;

    for (const auto& pair : midiProcessors)
    {
        if (pair.second == processor)
        {
            retval = pair.first;
            break;
        }
    }

    return retval;
}

int OscMappingManager::getNumAppMappings() const
{
    const ScopedLock sl(containerLock);
    return static_cast<int>(appMappings.size());
}

OscAppMapping* OscMappingManager::getAppMapping(int index)
{
    const ScopedLock sl(containerLock);
    int i = 0;

    for (auto& pair : appMappings)
    {
        if (i == index)
            return pair.second;
        ++i;
    }

    return nullptr;
}

StringArray OscMappingManager::getReceivedAddresses() const
{
    const ScopedLock sl(containerLock);
    return uniqueAddresses;
}

OscInput::OscInput()
{
    // Configure as no-audio, no-MIDI: remove default stereo buses.
    AudioProcessor::BusesLayout emptyLayout;
    setBusesLayout(emptyLayout);
}

OscInput::~OscInput() = default;

void OscInput::fillInPluginDescription(PluginDescription& description) const
{
    description.name = "OSC Input";
    description.descriptiveName = "Dummy AudioProcessor so we can see at a glance which plugins have OSC mappings.";
    description.pluginFormatName = "Internal";
    description.category = "Internal";
    description.manufacturerName = "Niall Moody";
    description.version = "1.00";
    description.uniqueId = description.name.hashCode();
    description.isInstrument = false;
    description.numInputChannels = 0;
    description.numOutputChannels = 0;
}
