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
class OscMapping : public Mapping {
  public:
    /// Creates an OSC mapping for the given plugin parameter and OSC address.
    ///
    /// @param manager The OSC mapping manager that owns this mapping.
    /// @param graph The filter graph the mapping belongs to.
    /// @param pluginId The id of the plugin this mapping applies to.
    /// @param param The index of the plugin parameter this mapping controls.
    /// @param oscAddress The OSC address this mapping responds to.
    /// @param oscParam The OSC argument index this mapping listens to.
    OscMapping(OscMappingManager* manager, FilterGraph* graph, uint32 pluginId, int param, const String& oscAddress,
               int oscParam);
    /// Loads an OscMapping from an XmlElement.
    ///
    /// @param manager The OSC mapping manager that owns this mapping.
    /// @param graph The filter graph the mapping belongs to.
    /// @param e The XmlElement to load the mapping from.
    OscMapping(OscMappingManager* manager, FilterGraph* graph, XmlElement* e);
    /// Unregisters this mapping from the manager.
    ~OscMapping() override;

    /// Called when an OSC message matching this mapping's address is received.
    ///
    /// @param val The normalised value (0.0 to 1.0) received via OSC.
    void messageReceived(float val);

    /// Returns an XmlElement representing this Mapping.
    ///
    /// @return A new XmlElement encoding this mapping's state.
    XmlElement* getXml() const override;

    /// Returns the OSC address this mapping responds to.
    const String& getAddress() const { return address; }
    /// Returns the OSC argument index this mapping listens to.
    int getParameterIndex() const { return parameter; }

    /// Sets the OSC address this mapping responds to.
    ///
    /// @param oscAddress The new OSC address for this mapping.
    void setAddress(const String& oscAddress);
    /// Sets the parameter index within the target plugin.
    ///
    /// @param val The new parameter index.
    void setParameterIndex(int val);

  private:
    OscMappingManager* mappingManager;
    String address;
    int parameter;
};

/// Mapping between an OSC message and an ApplicationCommandTarget.
class OscAppMapping {
  public:
    /// Creates an OSC-to-command mapping for the given address and command ID.
    ///
    /// @param manager The OSC mapping manager that owns this mapping.
    /// @param oscAddress The OSC address this mapping responds to.
    /// @param oscParam The OSC argument index this mapping listens to.
    /// @param commandId The application command ID to invoke when triggered.
    OscAppMapping(OscMappingManager* manager, const String& oscAddress, int oscParam, CommandID commandId);
    /// Loads an OscAppMapping from an XmlElement.
    ///
    /// @param manager The OSC mapping manager that owns this mapping.
    /// @param e The XmlElement to load the mapping from.
    OscAppMapping(OscMappingManager* manager, XmlElement* e);
    /// Unregisters this app mapping from the manager.
    ~OscAppMapping();

    /// Returns an XmlElement representing this mapping.
    ///
    /// @return A new XmlElement encoding this mapping's state.
    XmlElement* getXml() const;

    /// Returns the OSC address this mapping responds to.
    const String& getAddress() const { return address; }
    /// Returns the OSC argument index this mapping listens to.
    int getParameterIndex() const { return parameter; }
    /// Returns the command ID invoked when this mapping is triggered.
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
class OscMappingManager {
  public:
    /// Creates the manager.
    ///
    /// @param manager The app's ApplicationCommandManager used for invoking
    ///        application-level commands from OSC triggers.
    OscMappingManager(ApplicationCommandManager* manager);
    /// Destructor. Deletes all owned mappings.
    ~OscMappingManager();

    /// Called when a JUCE OSC message is received.
    ///
    /// Replaces the original messageReceived(OSC::Message*) with the JUCE
    /// juce_osc equivalent. Extracts float, int, and MIDI arguments and
    /// dispatches them to registered mappings.
    ///
    /// @param message The received JUCE OSC message.
    void messageReceived(const juce::OSCMessage& message);

    /// Dispatches a float OSC value to mappings at the given address.
    ///
    /// @param address The OSC address to dispatch to.
    /// @param index The OSC argument index to match against mapping parameter indices.
    /// @param val The float value to dispatch.
    void handleFloatMessage(const String& address, int index, float val);

    /// Dispatches a MIDI message extracted from OSC to registered MIDI processors.
    ///
    /// @param address The OSC address to dispatch to.
    /// @param midiMessage The MIDI message to dispatch.
    void handleMIDIMessage(const String& address, const juce::MidiMessage& midiMessage);

    /// Registers an OSC-to-parameter mapping at the given address.
    ///
    /// @param address The OSC address to register the mapping under.
    /// @param mapping The mapping to register.
    void registerMapping(const String& address, OscMapping* mapping);
    /// Unregisters an OSC-to-parameter mapping.
    ///
    /// @param mapping The mapping to unregister.
    void unregisterMapping(OscMapping* mapping);

    /// Registers an OSC-to-command mapping for application-level actions.
    ///
    /// @param mapping The app mapping to register.
    void registerAppMapping(OscAppMapping* mapping);
    /// Unregisters an OSC-to-command mapping.
    ///
    /// @param mapping The app mapping to unregister.
    void unregisterAppMapping(OscAppMapping* mapping);

    /// Registers a plugin that wants MIDI messages over OSC at the given address.
    ///
    /// @param address The OSC address to register the processor under.
    /// @param processor The plugin processor to receive MIDI messages.
    void registerMIDIProcessor(const String& address, BypassableInstance* processor);
    /// Unregisters a plugin from MIDI-over-OSC.
    ///
    /// @param processor The plugin processor to unregister.
    void unregisterMIDIProcessor(BypassableInstance* processor);
    /// Returns the OSC address for the given plugin, or empty if none.
    ///
    /// @param processor The plugin processor to look up.
    ///
    /// @return The OSC address registered for the processor, or an empty string if none.
    const String getMIDIProcessorAddress(BypassableInstance* processor) const;

    /// Returns the number of registered application-level mappings.
    ///
    /// @return The count of registered app mappings.
    int getNumAppMappings() const;
    /// Returns the application-level mapping at the given index.
    ///
    /// @param i The index of the mapping to return.
    ///
    /// @return The app mapping at the given index, or nullptr if out of range.
    OscAppMapping* getAppMapping(int i);

    /// Returns all unique OSC addresses received so far (for address learning).
    ///
    /// @return A StringArray of all unique OSC addresses received.
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
class OscInput : public AudioPluginInstance {
  public:
    /// Configures the processor with no audio buses.
    OscInput();
    /// Destructor.
    ~OscInput() override;

    /// Fills in the plugin description shown on the graph canvas.
    ///
    /// @param description The PluginDescription to populate.
    void fillInPluginDescription(PluginDescription& description) const override;
    const String getName() const override { return "OSC Input"; }
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override {
        juce::ignoreUnused(sampleRate, estimatedSamplesPerBlock);
    }
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override {
        juce::ignoreUnused(buffer, midiMessages);
    }
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 0; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override { juce::ignoreUnused(index); }
    const String getProgramName(int index) override {
        juce::ignoreUnused(index);
        return {};
    }
    void changeProgramName(int index, const String& newName) override { juce::ignoreUnused(index, newName); }
    void getStateInformation(juce::MemoryBlock& destData) override { juce::ignoreUnused(destData); }
    void setStateInformation(const void* data, int sizeInBytes) override { juce::ignoreUnused(data, sizeInBytes); }

    /// No audio buses.
    ///
    /// @param layouts The bus layout to test.
    ///
    /// @return True if the layout has no input and no output channels.
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override {
        return layouts.getMainInputChannels() == 0 && layouts.getMainOutputChannels() == 0;
    }
};

#endif
