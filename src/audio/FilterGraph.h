// FilterGraph.h - AudioProcessorGraph wrapper for the plugin signal chain.
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

#ifndef FILTERGRAPH_H_
#define FILTERGRAPH_H_

#include <JuceHeader.h>

class FilterGraph;

/// File extension used for saved filter graph documents.
const char* const filenameSuffix = ".filtergraph";
/// Wildcard pattern for filtering graph files in file dialogs.
const char* const filenameWildcard = "*.filtergraph";

/// A collection of filters (plugins) and connections between them.
///
/// Wraps JUCE's AudioProcessorGraph to manage the plugin signal chain.
/// Provides methods for adding/removing plugins, connecting/disconnecting
/// pins, and serializing/deserializing the graph to XML.
class FilterGraph : public FileBasedDocument {
  public:
    /// Creates a new graph with default audio I/O and MIDI input nodes.
    FilterGraph();
    /// Destructor. Clears all nodes from the graph.
    ~FilterGraph() override;

    /// Returns the underlying AudioProcessorGraph.
    AudioProcessorGraph& getGraph() { return graph; }

    /// Returns the number of filters (plugins) in the graph.
    int getNumFilters() const;
    /// Returns the node at the given index.
    ///
    /// @param index The index of the node to retrieve.
    /// @return The node at the given index.
    AudioProcessorGraph::Node::Ptr getNode(int index) const;
    /// Returns the node with the given UID, or nullptr if not found.
    ///
    /// @param uid The node UID to look up.
    /// @return The node matching the UID, or nullptr if not found.
    AudioProcessorGraph::Node::Ptr getNodeForId(AudioProcessorGraph::NodeID uid) const;

    /// Adds a plugin from a PluginDescription at the given canvas position.
    ///
    /// @param desc The plugin description to create the instance from.
    /// @param x The horizontal canvas position for the new node.
    /// @param y The vertical canvas position for the new node.
    void addFilter(const PluginDescription* desc, double x, double y);
    /// Adds an already-created plugin instance at the given canvas position.
    ///
    /// @param plugin The plugin instance to add; ownership is transferred.
    /// @param x The horizontal canvas position for the new node.
    /// @param y The vertical canvas position for the new node.
    void addFilter(std::unique_ptr<AudioPluginInstance> plugin, double x, double y);
    /// Adds an already-created internal processor at the given canvas position.
    ///
    /// @param plugin The internal processor to add; ownership is transferred.
    /// @param x The horizontal canvas position for the new node.
    /// @param y The vertical canvas position for the new node.
    void addFilter(std::unique_ptr<AudioProcessor> plugin, double x, double y);

    /// Removes the plugin with the given node ID.
    ///
    /// @param filterUID The node ID of the plugin to remove.
    void removeFilter(AudioProcessorGraph::NodeID filterUID);
    /// Disconnects all connections to/from the given node.
    ///
    /// @param filterUID The node ID of the plugin to disconnect.
    void disconnectFilter(AudioProcessorGraph::NodeID filterUID);

    /// Removes any connections that are no longer valid.
    void removeIllegalConnections();

    /// Sets the canvas position of a node.
    ///
    /// @param nodeId The node ID whose position to set.
    /// @param x The horizontal canvas position (clamped to 0-1).
    /// @param y The vertical canvas position (clamped to 0-1).
    void setNodePosition(AudioProcessorGraph::NodeID nodeId, double x, double y);
    /// Gets the canvas position of a node.
    ///
    /// @param nodeId The node ID whose position to retrieve.
    /// @param x Output parameter for the horizontal canvas position.
    /// @param y Output parameter for the vertical canvas position.
    void getNodePosition(AudioProcessorGraph::NodeID nodeId, double& x, double& y) const;

    /// Returns all connections in the graph.
    std::vector<AudioProcessorGraph::Connection> getConnections() const;

    /// Returns true if a connection exists between the given pins.
    ///
    /// @param sourceFilterUID The source node ID.
    /// @param sourceFilterChannel The source channel index.
    /// @param destFilterUID The destination node ID.
    /// @param destFilterChannel The destination channel index.
    /// @return True if a connection exists between the specified pins.
    bool connectionExists(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                          AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel) const;

    /// Returns true if a connection can be made between the given pins.
    ///
    /// @param sourceFilterUID The source node ID.
    /// @param sourceFilterChannel The source channel index.
    /// @param destFilterUID The destination node ID.
    /// @param destFilterChannel The destination channel index.
    /// @return True if a connection can be made between the specified pins.
    bool canConnect(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                    AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel) const;

    /// Adds a connection between the given pins.
    ///
    /// @param sourceFilterUID The source node ID.
    /// @param sourceFilterChannel The source channel index.
    /// @param destFilterUID The destination node ID.
    /// @param destFilterChannel The destination channel index.
    /// @return True if the connection was successfully added.
    bool addConnection(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                       AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel);

    /// Removes a connection between the given pins.
    ///
    /// @param sourceFilterUID The source node ID.
    /// @param sourceFilterChannel The source channel index.
    /// @param destFilterUID The destination node ID.
    /// @param destFilterChannel The destination channel index.
    void removeConnection(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                          AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel);

    /// Clears the graph, optionally re-adding default I/O nodes.
    ///
    /// @param addAudioIn Whether to re-add the audio input node.
    /// @param addMidiIn Whether to re-add the MIDI input node.
    /// @param addAudioOut Whether to re-add the audio output node.
    void clear(bool addAudioIn = true, bool addMidiIn = true, bool addAudioOut = true);

    /// Serializes the graph to XML.
    ///
    /// @return An XmlElement representing the full graph state.
    std::unique_ptr<XmlElement> createXml() const;
    /// Restores the graph from XML.
    ///
    /// @param xml The XmlElement to restore the graph from.
    void restoreFromXml(const XmlElement& xml);

    // FileBasedDocument overrides
    /// Returns the title used for the document window.
    String getDocumentTitle() override;
    /// Loads the graph from the given file.
    ///
    /// @param file The file to load the graph from.
    /// @return Result::ok() on success, or an error result on failure.
    Result loadDocument(const File& file) override;
    /// Saves the graph to the given file.
    ///
    /// @param file The file to save the graph to.
    /// @return Result::ok() on success, or an error result on failure.
    Result saveDocument(const File& file) override;
    /// Returns the most recently opened graph file.
    File getLastDocumentOpened() override;
    /// Records the given file as the most recently opened.
    ///
    /// @param file The file to record as most recently opened.
    void setLastDocumentOpened(const File& file) override;

    /// The special channel index used to refer to a filter's MIDI channel.
    static const int midiChannelNumber;

  private:
    /// The underlying JUCE audio processor graph.
    AudioProcessorGraph graph;
    /// Plays the graph through the audio device.
    AudioProcessorPlayer player;

    /// Counter for generating unique node UIDs.
    uint32 lastUID;
    /// Returns the next unique UID and increments the counter.
    uint32 getNextUID() { return ++lastUID; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterGraph)
};

#endif
