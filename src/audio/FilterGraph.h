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

const char* const filenameSuffix = ".filtergraph";
const char* const filenameWildcard = "*.filtergraph";

/// A collection of filters (plugins) and connections between them.
///
/// Wraps JUCE's AudioProcessorGraph to manage the plugin signal chain.
/// Provides methods for adding/removing plugins, connecting/disconnecting
/// pins, and serializing/deserializing the graph to XML.
class FilterGraph : public FileBasedDocument
{
  public:
    FilterGraph();
    ~FilterGraph() override;

    /// Returns the underlying AudioProcessorGraph.
    AudioProcessorGraph& getGraph() { return graph; }

    /// Returns the number of filters (plugins) in the graph.
    int getNumFilters() const;
    /// Returns the node at the given index.
    AudioProcessorGraph::Node::Ptr getNode(int index) const;
    /// Returns the node with the given UID, or nullptr if not found.
    AudioProcessorGraph::Node::Ptr getNodeForId(AudioProcessorGraph::NodeID uid) const;

    /// Adds a plugin from a PluginDescription at the given canvas position.
    void addFilter(const PluginDescription* desc, double x, double y);
    /// Adds an already-created plugin instance at the given canvas position.
    void addFilter(std::unique_ptr<AudioPluginInstance> plugin, double x, double y);
    /// Adds an already-created internal processor at the given canvas position.
    void addFilter(std::unique_ptr<AudioProcessor> plugin, double x, double y);

    /// Removes the plugin with the given node ID.
    void removeFilter(AudioProcessorGraph::NodeID filterUID);
    /// Disconnects all connections to/from the given node.
    void disconnectFilter(AudioProcessorGraph::NodeID filterUID);

    /// Removes any connections that are no longer valid.
    void removeIllegalConnections();

    /// Sets the canvas position of a node.
    void setNodePosition(AudioProcessorGraph::NodeID nodeId, double x, double y);
    /// Gets the canvas position of a node.
    void getNodePosition(AudioProcessorGraph::NodeID nodeId, double& x, double& y) const;

    /// Returns all connections in the graph.
    std::vector<AudioProcessorGraph::Connection> getConnections() const;

    /// Returns true if a connection exists between the given pins.
    bool connectionExists(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                          AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel) const;

    /// Returns true if a connection can be made between the given pins.
    bool canConnect(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                    AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel) const;

    /// Adds a connection between the given pins.
    bool addConnection(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                       AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel);

    /// Removes a connection between the given pins.
    void removeConnection(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                          AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel);

    /// Clears the graph, optionally re-adding default I/O nodes.
    void clear(bool addAudioIn = true, bool addMidiIn = true, bool addAudioOut = true);

    /// Serializes the graph to XML.
    std::unique_ptr<XmlElement> createXml() const;
    /// Restores the graph from XML.
    void restoreFromXml(const XmlElement& xml);

    // FileBasedDocument overrides
    /// Returns the title used for the document window.
    String getDocumentTitle() override;
    Result loadDocument(const File& file) override;
    Result saveDocument(const File& file) override;
    File getLastDocumentOpened() override;
    void setLastDocumentOpened(const File& file) override;

    /// The special channel index used to refer to a filter's MIDI channel.
    static const int midiChannelNumber;

  private:
    AudioProcessorGraph graph;
    AudioProcessorPlayer player;

    uint32 lastUID;
    uint32 getNextUID() { return ++lastUID; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterGraph)
};

#endif
