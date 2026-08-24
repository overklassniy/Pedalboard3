// OscMappingManager.h - Dispatches OSC messages to OscMappings.
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

#ifndef OSCMAPPINGMANAGER_H_
#define OSCMAPPINGMANAGER_H_

#include "Mapping.h"
#include "TapTempoHelper.h"

#include <JuceHeader.h>
#include <map>

class OscInput;
class OscMappingManager;
class BypassableInstance;

/// Mapping between an OSC address and a plugin parameter.
class OscMapping : public Mapping
{
  public:
    OscMapping(OscMappingManager* manager, FilterGraph* graph, uint32 pluginId, int param,
               const String& oscAddress, int oscParam);
    /// Constructor to load from an XmlElement.
    OscMapping(OscMappingManager* manager, FilterGraph* graph, XmlElement* e);
    ~OscMapping() override;

    /// Called when an OSC message matching this mapping's address is received.
    void messageReceived(float val);

    /// Returns an XmlElement representing this Mapping.
    XmlElement* getXml() const override;

    const String& getAddress() const { return address; }
    int getParameterIndex() const { return parameter; }

    /// Sets the OSC address this mapping responds to.
    void setAddress(const String& oscAddress);
    /// Sets the parameter index within the target plugin.
    void setParameterIndex(int val);

  private:
    OscMappingManager* mappingManager;
    String address;
    int parameter;
};

/// Mapping between an OSC message and an ApplicationCommandTarget.
class OscAppMapping
{
  public:
    OscAppMapping(OscMappingManager* manager, const String& oscAddress, int oscParam, CommandID commandId);
    OscAppMapping(OscMappingManager* manager, XmlElement* e);
    ~OscAppMapping();

    XmlElement* getXml() const;

    const String& getAddress() const { return address; }
    int getParameterIndex() const { return parameter; }
    CommandID getId() const { return id; }

  private:
    OscMappingManager* oscManager;
    String address;
    int parameter;
    CommandID id;
};

/// Dispatches OSC messages to OscMappings.
///
/// Replaces the original NiallsOSCLib-based implementation with JUCE's
/// juce_osc module. Preserves the original mapping semantics:
/// - Address learning (tracks unique received addresses).
/// - Multiple OSC values per message (indexed parameter).
/// - MIDI-over-OSC (MIDI bytes delivered to registered BypassableInstance).
/// - Mapping to plugin parameters and application commands.
class OscMappingManager
{
  public:
    ///
    /// @param manager The app's ApplicationCommandManager for invoking commands.
    OscMappingManager(ApplicationCommandManager* manager);
    /// Destructor. Deletes all owned mappings.
    ~OscMappingManager();

    /// Called when a JUCE OSC message is received.
    ///
    /// Replaces the original messageReceived(OSC::Message*) with the JUCE
    /// juce_osc equivalent. Extracts float, int, and MIDI arguments and
    /// dispatches them to registered mappings.
    void messageReceived(const juce::OSCMessage& message);

    /// Dispatches a float OSC value to mappings at the given address.
    void handleFloatMessage(const String& address, int index, float val);

    /// Dispatches a MIDI message extracted from OSC to registered MIDI processors.
    void handleMIDIMessage(const String& address, const juce::MidiMessage& midiMessage);

    /// Registers an OSC-to-parameter mapping at the given address.
    void registerMapping(const String& address, OscMapping* mapping);
    /// Unregisters an OSC-to-parameter mapping.
    void unregisterMapping(OscMapping* mapping);

    /// Registers an OSC-to-command mapping for application-level actions.
    void registerAppMapping(OscAppMapping* mapping);
    /// Unregisters an OSC-to-command mapping.
    void unregisterAppMapping(OscAppMapping* mapping);

    /// Registers a plugin that wants MIDI messages over OSC at the given address.
    void registerMIDIProcessor(const String& address, BypassableInstance* processor);
    /// Unregisters a plugin from MIDI-over-OSC.
    void unregisterMIDIProcessor(BypassableInstance* processor);
    /// Returns the OSC address for the given plugin, or empty if none.
    const String getMIDIProcessorAddress(BypassableInstance* processor) const;

    /// Returns the number of registered application-level mappings.
    int getNumAppMappings() const;
    /// Returns the application-level mapping at the given index.
    OscAppMapping* getAppMapping(int i);

    /// Returns all unique OSC addresses received so far (for address learning).
    StringArray getReceivedAddresses() const;

  private:
    /// Protects all containers against concurrent access from the OSC
    /// network thread (reads) and the message thread (mutations).
    mutable juce::CriticalSection containerLock;

    std::multimap<String, OscMapping*> mappings;
    std::multimap<String, OscAppMapping*> appMappings;
    ApplicationCommandManager* appManager;
    std::multimap<String, BypassableInstance*> midiProcessors;
    StringArray uniqueAddresses;

    TapTempoHelper tapHelper;
};

/// Dummy AudioProcessor representing the OSC input node on the graph canvas.
///
/// This is a visual placeholder so users can see which plugins have OSC
/// mappings. The actual OSC reception is handled by the OscMappingManager
/// and a juce::OSCReceiver (created in MainPanel).
class OscInput : public AudioPluginInstance
{
  public:
    OscInput();
    ~OscInput() override;

    void fillInPluginDescription(PluginDescription& description) const override;
    const String getName() const override { return "OSC Input"; }
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override { juce::ignoreUnused(sampleRate, estimatedSamplesPerBlock); }
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override { juce::ignoreUnused(buffer, midiMessages); }
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 0; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override { juce::ignoreUnused(index); }
    const String getProgramName(int index) override { juce::ignoreUnused(index); return {}; }
    void changeProgramName(int index, const String& newName) override { juce::ignoreUnused(index, newName); }
    void getStateInformation(juce::MemoryBlock& destData) override { juce::ignoreUnused(destData); }
    void setStateInformation(const void* data, int sizeInBytes) override { juce::ignoreUnused(data, sizeInBytes); }

    /// No audio buses.
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
        return layouts.getMainInputChannels() == 0 && layouts.getMainOutputChannels() == 0;
    }
};

#endif
