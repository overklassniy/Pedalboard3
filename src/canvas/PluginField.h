// PluginField.h - Field representing the signal path through the app.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2009 Niall Moody.
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

#ifndef PLUGINFIELD_H_
#define PLUGINFIELD_H_

#include "MidiMappingManager.h"
#include "OscMappingManager.h"

#include <JuceHeader.h>
#include <map>

class Mapping;
class FilterGraph;
class PluginConnection;
class PluginPinComponent;

/// Field representing the signal path through the app.
///
/// It's a ChangeBroadcaster, and broadcasts a change message whenever
/// something changes (i.e. so MainPanel can call FileBaseDocument::changed()
/// accordingly).
class PluginField : public juce::Component,
                    public juce::ChangeBroadcaster,
                    public juce::ChangeListener,
                    public juce::FileDragAndDropTarget,
                    public juce::AudioPlayHead,
                    public juce::Timer {
  public:
    /// Constructs the field from the given graph, plugin list, and command manager.
    ///
    /// @param filterGraph The signal path graph for the field.
    /// @param list The list of available plugins for the plugin picker menu.
    /// @param appManager The application command manager used by mapping managers.
    PluginField(FilterGraph* filterGraph, juce::KnownPluginList* list, juce::ApplicationCommandManager* appManager);
    /// Destroys the field, detaching from any Viewport first.
    ~PluginField() override;

    /// Fills in the background.
    void paint(juce::Graphics& g) override;

    /// Used to add plugins with a double-click.
    void mouseDown(const juce::MouseEvent& e) override;

    /// So we're informed when PluginComponents are moved around, and can
    /// update our bounds accordingly.
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    /// Used to periodically update PluginComponents etc.
    void timerCallback() override;

    /// Used to accept dragged plugin files.
    ///
    /// @param files The list of file paths being dragged.
    /// @return True if any of the files are plugin or sound files, false otherwise.
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    /// Used to accept dragged plugin files.
    ///
    /// @param files The list of dropped file paths.
    /// @param x The x coordinate where the files were dropped.
    /// @param y The y coordinate where the files were dropped.
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    /// So plugins get info about the tempo etc.
    ///
    /// JUCE 8 AudioPlayHead uses Optional<PositionInfo> instead of the
    /// deprecated CurrentPositionInfo struct.
    ///
    /// @return The current playback position info, or nullopt if unavailable.
    juce::Optional<juce::AudioPlayHead::PositionInfo> getPosition() const override;

    /// Enables/disables the audio input.
    ///
    /// @param val True to enable the audio input, false to disable and remove it.
    void enableAudioInput(bool val);
    /// Enables/disables the MIDI input.
    ///
    /// @param val True to enable the MIDI input, false to disable and remove it.
    void enableMidiInput(bool val);
    /// Enables/disables the OSC input.
    ///
    /// @param val True to enable the OSC input, false to disable and remove it.
    void enableOscInput(bool val);

    /// Sets whether to automatically open the mappings window or not.
    void setAutoMappingsWindow(bool val);

    /// Sets the current tempo.
    void setTempo(double val);
    /// Returns the current tempo.
    double getTempo() const { return tempo; }

    /// Adds a filter to the field.
    ///
    /// @param index The index of the filter in the FilterGraph.
    /// @param broadcastChangeMessage Whether to send a change message after adding the filter.
    void addFilter(int index, bool broadcastChangeMessage = true);
    /// Deletes a filter from the field.
    ///
    /// @param node The graph node to delete.
    void deleteFilter(AudioProcessorGraph::Node::Ptr node);

    /// Lets us know the user's edited a processor name.
    ///
    /// @param id The uid of the processor whose name was edited.
    /// @param val The new name for the processor.
    void updateProcessorName(uint32 id, const juce::String& val);

    /// Returns the FilterGraph.
    FilterGraph* getFilterGraph() { return signalPath; }

    /// Adds a connection to the field.
    ///
    /// @param source The source pin to start the connection from.
    /// @param connectAll Whether to connect all outputs from the source.
    void addConnection(PluginPinComponent* source, bool connectAll);
    /// Drags a connection between plugins.
    ///
    /// @param x The x coordinate to drag to.
    /// @param y The y coordinate to drag to.
    void dragConnection(int x, int y);
    /// Makes a connection between two plugins, or deletes it.
    ///
    /// @param x The x coordinate where the mouse was released.
    /// @param y The y coordinate where the mouse was released.
    void releaseConnection(int x, int y);
    /// Deletes any selected connections.
    void deleteConnection();

    /// Enables/disables the Midi Input->plugin connection for the passed-in
    /// Node.
    ///
    /// @param node The node to enable/disable MIDI for.
    /// @param val If true, removes the MIDI Input-to-node connection; if false, adds it.
    void enableMidiForNode(AudioProcessorGraph::Node::Ptr node, bool val);
    /// Returns true if the Node has a Midi Input->plugin connection.
    ///
    /// @param node The node to check for a MIDI connection.
    /// @return True if the node has a Midi Input->plugin connection, false otherwise.
    bool getMidiEnabledForNode(AudioProcessorGraph::Node::Ptr node) const;

    /// Adds a Mapping.
    ///
    /// @param mapping The Mapping to add.
    void addMapping(Mapping* mapping);
    /// Removes a Mapping.
    ///
    /// Also deletes mapping.
    ///
    /// @param mapping The Mapping to remove and delete.
    void removeMapping(Mapping* mapping);
    /// Returns an Array of all the Mappings for the passed-in plugin id.
    ///
    /// @param id The plugin id to get mappings for.
    /// @return An Array of all Mappings associated with the given plugin id.
    juce::Array<Mapping*> getMappingsForPlugin(uint32 id);

    /// Returns the MidiMappingManager.
    MidiMappingManager* getMidiManager() { return &midiManager; }
    /// Returns the OscMappingManager.
    OscMappingManager* getOscManager() { return &oscManager; }

    /// Called when the app receives data on its OSC port.
    ///
    /// @param data Pointer to the received data.
    /// @param dataSize The number of bytes of data received.
    void socketDataArrived(char* data, int32 dataSize);

    /// Returns the XML for the current patch.
    ///
    /// @return An XmlElement representing the current patch, or nullptr on failure.
    std::unique_ptr<juce::XmlElement> getXml() const;
    /// Loads a new patch from an XmlElement.
    ///
    /// @param patch The XmlElement containing the patch data, or nullptr to clear the field.
    void loadFromXml(juce::XmlElement* patch);

    /// Clears the field.
    void clear();

    /// Clears the 'double-click...' message.
    void clearDoubleClickMessage();

  private:
    /// Helper method. Clears mappings.
    void clearMappings();
    /// Helper method. Handles a single OSC bundle.
    ///
    /// @param bundle The OSC bundle to handle.
    void handleOscBundle(const juce::OSCBundle& bundle);

    /// Helper method. Makes sure PluginConnections are always behind
    /// PluginComponents.
    void moveConnectionsBehind();
    /// Used to find whether a Plugin Connection has been dragged to a
    /// PluginPinComponent.
    ///
    /// @param x The x coordinate to check, relative to this field.
    /// @param y The y coordinate to check, relative to this field.
    /// @return The PluginPinComponent at the given coordinates, this field if the point is on the field itself, or
    /// nullptr if outside.
    juce::Component* getPinAt(const int x, const int y);

    /// Helper method. Connects all outputs from a source to all inputs of a
    /// destination.
    ///
    /// @param connection The connection whose source and destination pins determine the starting points for connecting
    /// all remaining outputs to inputs.
    void connectAll(PluginConnection* connection);

    /// The signal path itself.
    FilterGraph* signalPath;
    /// The list of possible plugins.
    juce::KnownPluginList* pluginList;
    /// The mappings for this field.
    std::multimap<uint32, Mapping*> mappings;
    /// The manager for any MidiMappings.
    MidiMappingManager midiManager;
    /// The manager for any OscMappings.
    OscMappingManager oscManager;

    /// Any user-edited processor names.
    std::map<uint32, juce::String> userNames;

    /// Temporary PluginConnection for dragging.
    PluginConnection* draggingConnection;

    /// The current tempo in bpm.
    double tempo;

    /// Whether to display the 'double-click...' message or not.
    bool displayDoubleClickMessage;

    /// Whether the audio input is enabled or not.
    bool audioInputEnabled;
    /// Whether the MIDI input is enabled or not.
    bool midiInputEnabled;
    /// Whether the OSC input is enabled or not.
    bool oscInputEnabled;

    /// Whether to open the mappings window when a param connection is made.
    bool autoMappingsWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginField)
};

#endif
