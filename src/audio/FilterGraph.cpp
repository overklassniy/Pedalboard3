// FilterGraph.cpp - AudioProcessorGraph wrapper for the plugin signal chain.
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

#include "FilterGraph.h"

#include "AudioSingletons.h"
#include "BypassableInstance.h"
#include "InternalFilters.h"
#include "PropertiesSingleton.h"

const int FilterGraph::midiChannelNumber = 0x1000;

FilterGraph::FilterGraph()
    : FileBasedDocument(filenameSuffix, filenameWildcard, "Load a filter graph", "Save a filter graph"), lastUID(0) {
    InternalPluginFormat internalFormat;

    // Create default I/O nodes. Audio input and MIDI input are enabled by default.
    addFilter(internalFormat.getDescriptionFor(InternalPluginFormat::audioInputFilter), 10.0, 10.0);
    addFilter(internalFormat.getDescriptionFor(InternalPluginFormat::midiInputFilter), 10.0, 120.0);
    addFilter(internalFormat.getDescriptionFor(InternalPluginFormat::audioOutputFilter), 892.0, 10.0);

    setChangedFlag(false);
}

FilterGraph::~FilterGraph() {
    graph.clear();
}

int FilterGraph::getNumFilters() const {
    return graph.getNumNodes();
}

AudioProcessorGraph::Node::Ptr FilterGraph::getNode(int index) const {
    return graph.getNode(index);
}

AudioProcessorGraph::Node::Ptr FilterGraph::getNodeForId(AudioProcessorGraph::NodeID uid) const {
    return graph.getNodeForId(uid);
}

void FilterGraph::addFilter(const PluginDescription* desc, double x, double y) {
    if (desc == nullptr)
        return;

    String errorMessage;
    auto tempInstance = AudioPluginFormatManagerSingleton::getInstance().createPluginInstance(
        *desc, graph.getSampleRate(), graph.getBlockSize(), errorMessage);

    if (tempInstance == nullptr) {
        AlertWindow::showMessageBox(AlertWindow::WarningIcon, TRANS("Couldn't create filter"), errorMessage);
        return;
    }

    // AudioGraphIOProcessor nodes are not wrapped in BypassableInstance.
    // MidiInterceptor and OscInput will also bypass wrapping when they are ported.
    std::unique_ptr<AudioPluginInstance> instance;

    if (dynamic_cast<AudioProcessorGraph::AudioGraphIOProcessor*>(tempInstance.get()) != nullptr)
        instance = std::move(tempInstance);
    else
        instance = std::make_unique<BypassableInstance>(std::move(tempInstance));

    auto node = graph.addNode(std::move(instance));

    if (node != nullptr) {
        node->properties.set("x", x);
        node->properties.set("y", y);
        changed();
    }
}

void FilterGraph::addFilter(std::unique_ptr<AudioPluginInstance> plugin, double x, double y) {
    if (plugin == nullptr)
        return;

    auto instance = std::make_unique<BypassableInstance>(std::move(plugin));
    auto node = graph.addNode(std::move(instance));

    if (node != nullptr) {
        node->properties.set("x", x);
        node->properties.set("y", y);
        changed();
    } else {
        AlertWindow::showMessageBox(AlertWindow::WarningIcon, TRANS("Couldn't create filter"),
                                    "Failed to add plugin to graph");
    }
}

void FilterGraph::addFilter(std::unique_ptr<AudioProcessor> plugin, double x, double y) {
    if (plugin == nullptr)
        return;

    auto node = graph.addNode(std::move(plugin));

    if (node != nullptr) {
        node->properties.set("x", x);
        node->properties.set("y", y);
        changed();
    } else {
        AlertWindow::showMessageBox(AlertWindow::WarningIcon, TRANS("Couldn't create filter"),
                                    "Failed to add plugin to graph");
    }
}

void FilterGraph::removeFilter(AudioProcessorGraph::NodeID id) {
    if (graph.removeNode(id))
        changed();
}

void FilterGraph::disconnectFilter(AudioProcessorGraph::NodeID id) {
    if (graph.disconnectNode(id))
        changed();
}

void FilterGraph::removeIllegalConnections() {
    if (graph.removeIllegalConnections())
        changed();
}

void FilterGraph::setNodePosition(AudioProcessorGraph::NodeID nodeId, double x, double y) {
    auto n = graph.getNodeForId(nodeId);

    if (n != nullptr) {
        n->properties.set("x", jlimit(0.0, 1.0, x));
        n->properties.set("y", jlimit(0.0, 1.0, y));
    }
}

void FilterGraph::getNodePosition(AudioProcessorGraph::NodeID nodeId, double& x, double& y) const {
    x = y = 0;

    auto n = graph.getNodeForId(nodeId);

    if (n != nullptr) {
        x = n->properties.getWithDefault("x", 0.0);
        y = n->properties.getWithDefault("y", 0.0);
    }
}

std::vector<AudioProcessorGraph::Connection> FilterGraph::getConnections() const {
    return graph.getConnections();
}

bool FilterGraph::connectionExists(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                                   AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel) const {
    return graph.isConnected({{sourceFilterUID, sourceFilterChannel}, {destFilterUID, destFilterChannel}});
}

bool FilterGraph::canConnect(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                             AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel) const {
    return graph.canConnect({{sourceFilterUID, sourceFilterChannel}, {destFilterUID, destFilterChannel}});
}

bool FilterGraph::addConnection(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                                AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel) {
    auto result = graph.addConnection({{sourceFilterUID, sourceFilterChannel}, {destFilterUID, destFilterChannel}});

    if (result)
        changed();

    return result;
}

void FilterGraph::removeConnection(AudioProcessorGraph::NodeID sourceFilterUID, int sourceFilterChannel,
                                   AudioProcessorGraph::NodeID destFilterUID, int destFilterChannel) {
    if (graph.removeConnection({{sourceFilterUID, sourceFilterChannel}, {destFilterUID, destFilterChannel}}))
        changed();
}

void FilterGraph::clear(bool addAudioIn, bool addMidiIn, bool addAudioOut) {
    InternalPluginFormat internalFormat;

    graph.clear();

    if (addAudioIn)
        addFilter(internalFormat.getDescriptionFor(InternalPluginFormat::audioInputFilter), 10.0, 10.0);

    if (addMidiIn)
        addFilter(internalFormat.getDescriptionFor(InternalPluginFormat::midiInputFilter), 10.0, 120.0);

    if (addAudioOut)
        addFilter(internalFormat.getDescriptionFor(InternalPluginFormat::audioOutputFilter), 892.0, 10.0);

    changed();
}

String FilterGraph::getDocumentTitle() {
    if (!getFile().exists())
        return "Unnamed";

    return getFile().getFileNameWithoutExtension();
}

Result FilterGraph::loadDocument(const File& file) {
    auto xml = XmlDocument::parse(file);

    if (xml == nullptr || !xml->hasTagName("FILTERGRAPH"))
        return Result::fail("Not a valid filter graph file");

    restoreFromXml(*xml);
    return Result::ok();
}

Result FilterGraph::saveDocument(const File& file) {
    auto xml = createXml();

    if (xml == nullptr || !xml->writeTo(file))
        return Result::fail("Couldn't write to the file");

    return Result::ok();
}

File FilterGraph::getLastDocumentOpened() {
    RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString(
        PropertiesSingleton::getInstance().getUserSettings()->getValue("recentFilterGraphFiles"));

    return recentFiles.getFile(0);
}

void FilterGraph::setLastDocumentOpened(const File& file) {
    RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString(
        PropertiesSingleton::getInstance().getUserSettings()->getValue("recentFilterGraphFiles"));

    recentFiles.addFile(file);

    PropertiesSingleton::getInstance().getUserSettings()->setValue("recentFilterGraphFiles", recentFiles.toString());
}

/// Serializes a single graph node to an XML element.
///
/// Writes the node's UID, canvas position, editor window state, current program,
/// plugin description, and saved state. Returns nullptr if the node's processor
/// is not an AudioPluginInstance.
///
/// @param node The graph node to serialize.
/// @return An XmlElement representing the node, or nullptr if the node's
///         processor is not an AudioPluginInstance.
static std::unique_ptr<XmlElement> createNodeXml(AudioProcessorGraph::Node* const node) {
    auto* plugin = dynamic_cast<AudioPluginInstance*>(node->getProcessor());
    if (plugin == nullptr) {
        jassertfalse;
        return nullptr;
    }

    auto e = std::make_unique<XmlElement>("FILTER");
    e->setAttribute("uid", static_cast<int>(node->nodeID.uid));
    e->setAttribute("x", static_cast<double>(node->properties.getWithDefault("x", 0.0)));
    e->setAttribute("y", static_cast<double>(node->properties.getWithDefault("y", 0.0)));
    e->setAttribute("uiLastX", static_cast<int>(node->properties.getWithDefault("uiLastX", 0)));
    e->setAttribute("uiLastY", static_cast<int>(node->properties.getWithDefault("uiLastY", 0)));
    e->setAttribute("windowOpen", static_cast<bool>(node->properties.getWithDefault("windowOpen", false)));
    e->setAttribute("program", node->getProcessor()->getCurrentProgram());

    PluginDescription pd;
    plugin->fillInPluginDescription(pd);
    e->addChildElement(pd.createXml().release());

    auto state = std::make_unique<XmlElement>("STATE");
    MemoryBlock m;
    node->getProcessor()->getStateInformation(m);
    state->addTextElement(m.toBase64Encoding());
    e->addChildElement(state.release());

    return e;
}

std::unique_ptr<XmlElement> FilterGraph::createXml() const {
    auto xml = std::make_unique<XmlElement>("FILTERGRAPH");

    for (int i = 0; i < graph.getNumNodes(); ++i) {
        auto nodeXml = createNodeXml(graph.getNode(i).get());
        if (nodeXml != nullptr)
            xml->addChildElement(nodeXml.release());
    }

    auto connections = graph.getConnections();
    for (const auto& fc : connections) {
        auto e = std::make_unique<XmlElement>("CONNECTION");
        e->setAttribute("srcFilter", static_cast<int>(fc.source.nodeID.uid));
        e->setAttribute("srcChannel", fc.source.channelIndex);
        e->setAttribute("dstFilter", static_cast<int>(fc.destination.nodeID.uid));
        e->setAttribute("dstChannel", fc.destination.channelIndex);
        xml->addChildElement(e.release());
    }

    return xml;
}

void FilterGraph::restoreFromXml(const XmlElement& xml) {
    clear(false, false, false);

    for (auto* e : xml.getChildIterator()) {
        if (e->getTagName() == "FILTER") {
            String errorMessage;
            PluginDescription pd;

            for (auto* child : e->getChildIterator()) {
                if (pd.loadFromXml(*child))
                    break;
            }

            auto tempInstance = AudioPluginFormatManagerSingleton::getInstance().createPluginInstance(
                pd, graph.getSampleRate(), graph.getBlockSize(), errorMessage);
            if (tempInstance == nullptr)
                continue;

            std::unique_ptr<AudioPluginInstance> instance;
            if (dynamic_cast<AudioProcessorGraph::AudioGraphIOProcessor*>(tempInstance.get()) != nullptr)
                instance = std::move(tempInstance);
            else
                instance = std::make_unique<BypassableInstance>(std::move(tempInstance));

            auto node = graph.addNode(std::move(instance),
                                      AudioProcessorGraph::NodeID(static_cast<uint32>(e->getIntAttribute("uid"))));

            if (node != nullptr) {
                auto* state = e->getChildByName("STATE");
                if (state != nullptr) {
                    MemoryBlock m;
                    m.fromBase64Encoding(state->getAllSubText());
                    node->getProcessor()->setStateInformation(m.getData(), static_cast<int>(m.getSize()));
                }

                node->properties.set("x", e->getDoubleAttribute("x"));
                node->properties.set("y", e->getDoubleAttribute("y"));
                node->properties.set("uiLastX", e->getIntAttribute("uiLastX"));
                node->properties.set("uiLastY", e->getIntAttribute("uiLastY"));
                node->properties.set("windowOpen", e->getIntAttribute("windowOpen"));

                node->getProcessor()->setCurrentProgram(e->getIntAttribute("program"));
                changed();
            }
        }
    }

    for (auto* e2 : xml.getChildIterator()) {
        if (e2->getTagName() == "CONNECTION") {
            addConnection(AudioProcessorGraph::NodeID(static_cast<uint32>(e2->getIntAttribute("srcFilter"))),
                          e2->getIntAttribute("srcChannel"),
                          AudioProcessorGraph::NodeID(static_cast<uint32>(e2->getIntAttribute("dstFilter"))),
                          e2->getIntAttribute("dstChannel"));
        }
    }

    graph.removeIllegalConnections();
}
