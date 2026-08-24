// PluginField.cpp - Field representing the signal path through the app.
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

#include "PluginField.h"
#include "FilterGraph.h"
#include "PluginComponent.h"
#include "Mapping.h"
#include "LogFile.h"
#include "InternalFilters.h"
#include "PropertiesSingleton.h"
#include "ColourScheme.h"
#include "MainTransport.h"
#include "PedalboardProcessors.h"
#include "BypassableInstance.h"
#include "AudioSingletons.h"

#include <juce_osc/juce_osc.h>

PluginField::PluginField(FilterGraph *filterGraph,
                         juce::KnownPluginList *list,
                         juce::ApplicationCommandManager *appManager) :
    signalPath(filterGraph),
    pluginList(list),
    midiManager(appManager),
    oscManager(appManager),
    draggingConnection(nullptr),
    tempo(120.0),
    displayDoubleClickMessage(true)
{
    int i;

    audioInputEnabled = PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("AudioInput", true);
    midiInputEnabled = PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("MidiInput", true);
    oscInputEnabled = PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("OscInput", true);

    autoMappingsWindow = PropertiesSingleton::getInstance().getUserSettings()->getBoolValue("AutoMappingsWindow", true);

    // Inform the signal path about our AudioPlayHead.
    signalPath->getGraph().setPlayHead(this);

    // Add OSC input.
    if (oscInputEnabled)
    {
        OscInput p;
        juce::PluginDescription desc;

        p.fillInPluginDescription(desc);

        signalPath->addFilter(&desc, 10, 215);
    }

    // Setup gui.
    for (i = 0; i < signalPath->getNumFilters(); ++i)
        addFilter(i);

    // Add MidiInterceptor.
    if (midiInputEnabled)
    {
        MidiInterceptor p;
        juce::PluginDescription desc;

        p.fillInPluginDescription(desc);

        signalPath->addFilter(&desc, 100, 100);

        // And connect it up to the midi input.
        {
            AudioProcessorGraph::NodeID midiInput;
            AudioProcessorGraph::NodeID midiInterceptor;

            for (i = 0; i < signalPath->getNumFilters(); ++i)
            {
                if (signalPath->getNode(i)->getProcessor()->getName() == "Midi Input")
                    midiInput = signalPath->getNode(i)->nodeID;
                else if (signalPath->getNode(i)->getProcessor()->getName() == "Midi Interceptor")
                {
                    midiInterceptor = signalPath->getNode(i)->nodeID;
                    dynamic_cast<MidiInterceptor *>(signalPath->getNode(i)->getProcessor())->setManager(&midiManager);
                }
            }
            signalPath->addConnection(midiInput,
                                      AudioProcessorGraph::midiChannelIndex,
                                      midiInterceptor,
                                      AudioProcessorGraph::midiChannelIndex);
        }
    }

    setWantsKeyboardFocus(true);

    startTimer(50);
}

PluginField::~PluginField()
{
    int i;
    std::multimap<uint32, Mapping *>::iterator it;

    // JUCE 8: detach from any Viewport that might be viewing us before
    // destroying child components.
    if (auto* vp = findParentComponentOfClass<juce::Viewport>())
        vp->setViewedComponent(nullptr, false);

    // If we don't do this, the connections will try to contact their pins,
    // which may have already been deleted.
    for (i = (getNumChildComponents() - 1); i >= 0; --i)
    {
        PluginConnection *connection = dynamic_cast<PluginConnection *>(getChildComponent(i));

        if (connection)
        {
            removeChildComponent(connection);
            delete connection;
        }
    }

    for (it = mappings.begin(); it != mappings.end(); ++it)
        delete it->second;

    deleteAllChildren();
}

void PluginField::paint(juce::Graphics& g)
{
    g.fillAll(ColourScheme::getInstance().colours["Field Background"]);

    if (displayDoubleClickMessage)
    {
        g.setColour(ColourScheme::getInstance().colours["Text Colour"].withAlpha(0.5f));
        g.drawText("<double-click to add processor>",
                   400,
                   230,
                   300,
                   30,
                   juce::Justification(juce::Justification::centredLeft),
                   false);
    }
}

void PluginField::mouseDown(const juce::MouseEvent& e)
{
    if (e.getNumberOfClicks() == 2)
    {
        int result = 0;
        juce::PopupMenu menu;

        pluginList->addToMenu(menu, juce::KnownPluginList::sortAlphabetically);

        result = menu.show();

        if (result > 0)
        {
            int pluginIndex = signalPath->getNumFilters() - 1;

            signalPath->addFilter(pluginList->getType(pluginList->getIndexChosenByMenu(result)),
                                  static_cast<double>(e.x),
                                  static_cast<double>(e.y));

            // Make sure the plugin got created before we add a component for it.
            if ((signalPath->getNumFilters() - 1) > pluginIndex)
            {
                pluginIndex = signalPath->getNumFilters() - 1;

                addFilter(pluginIndex);

                sendChangeMessage();

                clearDoubleClickMessage();
            }
        }
    }
}

void PluginField::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    PluginComponent *pluginComp = dynamic_cast<PluginComponent *>(source);

    if (pluginComp)
    {
        juce::Point<int> fieldSize(getWidth(), getHeight());
        juce::Point<int> pluginPos = pluginComp->getPosition();
        juce::Point<int> pluginSize(pluginComp->getWidth(), pluginComp->getHeight());

        if ((pluginPos.getX() + pluginSize.getX()) > fieldSize.getX())
            fieldSize.setX((pluginPos.getX() + pluginSize.getX()));
        if ((pluginPos.getY() + pluginSize.getY()) > fieldSize.getY())
            fieldSize.setY((pluginPos.getY() + pluginSize.getY()));

        setSize(fieldSize.getX(), fieldSize.getY());
        repaint();
    }
}

void PluginField::timerCallback()
{
    int i;

    for (i = 0; i < getNumChildComponents(); ++i)
    {
        PluginComponent *comp = dynamic_cast<PluginComponent *>(getChildComponent(i));

        if (comp)
            comp->timerUpdate();
    }
}

bool PluginField::isInterestedInFileDrag(const juce::StringArray& files)
{
    int i;
    bool retval = false;

    for (i = 0; i < files.size(); ++i)
    {
        // If they're plugins.
#ifdef _WIN32
        if (files[i].endsWithIgnoreCase(".dll") || files[i].endsWithIgnoreCase(".vst3"))
        {
            retval = true;
            break;
        }
#elif defined(__APPLE__)
        if (files[i].endsWith(".vst") || files[i].endsWith(".vst3") || files[i].endsWith(".component"))
        {
            retval = true;
            break;
        }
#elif defined(__linux__)
        if (files[i].endsWith(".so") || files[i].endsWithIgnoreCase(".vst3"))
        {
            retval = true;
            break;
        }
#endif
        // If they're sound files.
        else if (files[i].endsWithIgnoreCase(".wav") ||
                 files[i].endsWithIgnoreCase(".aif") ||
                 files[i].endsWithIgnoreCase(".aiff") ||
                 files[i].endsWithIgnoreCase(".ogg") ||
                 files[i].endsWithIgnoreCase(".flac") ||
                 files[i].endsWithIgnoreCase(".wma"))
        {
            retval = true;
            break;
        }
    }

    return retval;
}

void PluginField::filesDropped(const juce::StringArray& files, int x, int y)
{
    int i;
    bool soundsInArray = false;
    bool pluginsInArray = false;
    juce::OwnedArray<juce::PluginDescription> foundPlugins;

    for (i = 0; i < files.size(); ++i)
    {
        // If they're plugins.
#ifdef _WIN32
        if (files[i].endsWithIgnoreCase(".dll") || files[i].endsWithIgnoreCase(".vst3"))
            pluginsInArray = true;
#elif defined(__APPLE__)
        if (files[i].endsWith(".vst") || files[i].endsWith(".vst3") || files[i].endsWith(".component"))
            pluginsInArray = true;
#elif defined(__linux__)
        if (files[i].endsWith(".so") || files[i].endsWithIgnoreCase(".vst3"))
            pluginsInArray = true;
#endif
        // If they're sound files.
        else if (files[i].endsWithIgnoreCase(".wav") ||
                 files[i].endsWithIgnoreCase(".aif") ||
                 files[i].endsWithIgnoreCase(".aiff") ||
                 files[i].endsWithIgnoreCase(".ogg") ||
                 files[i].endsWithIgnoreCase(".flac") ||
                 files[i].endsWithIgnoreCase(".wma"))
        {
            soundsInArray = true;
        }
    }

    if (pluginsInArray)
    {
        pluginList->scanAndAddDragAndDroppedFiles(AudioPluginFormatManagerSingleton::getInstance(),
                                                  files,
                                                  foundPlugins);

        for (i = 0; i < foundPlugins.size(); ++i)
        {
            int pluginIndex = signalPath->getNumFilters() - 1;

            signalPath->addFilter(foundPlugins[i],
                                  static_cast<double>(x),
                                  static_cast<double>(y));

            // Make sure the plugin got created before we add a component for it.
            if ((signalPath->getNumFilters() - 1) > pluginIndex)
            {
                pluginIndex = signalPath->getNumFilters() - 1;

                addFilter(pluginIndex);

                sendChangeMessage();
            }

            x += 100;
            y += 100;
        }
    }

    if (soundsInArray)
    {
        for (i = 0; i < files.size(); ++i)
        {
            int pluginIndex = signalPath->getNumFilters() - 1;

            signalPath->addFilter(std::unique_ptr<juce::AudioProcessor>(std::make_unique<FilePlayerProcessor>(juce::File(files[i]))),
                                  static_cast<double>(x),
                                  static_cast<double>(y));

            // Make sure the plugin got created before we add a component for it.
            if ((signalPath->getNumFilters() - 1) > pluginIndex)
            {
                pluginIndex = signalPath->getNumFilters() - 1;

                addFilter(pluginIndex);

                sendChangeMessage();

                clearDoubleClickMessage();
            }
        }
    }
}

juce::Optional<juce::AudioPlayHead::PositionInfo> PluginField::getPosition() const
{
    juce::AudioPlayHead::PositionInfo info;

    info.setBpm(tempo);
    {
        juce::AudioPlayHead::TimeSignature ts;
        ts.numerator = 4;
        ts.denominator = 4;
        info.setTimeSignature(ts);
    }
    info.setTimeInSeconds(0.0);
    info.setEditOriginTime(0.0);
    info.setPpqPosition(0.0);
    info.setPpqPositionOfLastBarStart(0.0);
    info.setIsPlaying(MainTransport::getInstance()->getState());
    info.setIsRecording(false);

    return info;
}

void PluginField::enableAudioInput(bool val)
{
    int i;

    audioInputEnabled = val;

    if (!val)
    {
        // Delete the filter(s) in the signal path.
        for (i = (signalPath->getNumFilters() - 1); i >= 0; --i)
        {
            if (signalPath->getNode(i)->getProcessor()->getName() == "Audio Input")
                deleteFilter(signalPath->getNode(i));
        }

        // Delete the associated "Audio Input" PluginComponent(s).
        for (i = (getNumChildComponents() - 1); i >= 0; --i)
        {
            PluginComponent *comp = dynamic_cast<PluginComponent *>(getChildComponent(i));

            if (comp)
            {
                if (comp->getNode()->getProcessor()->getName() == "Audio Input")
                    delete removeChildComponent(i);
            }
        }
    }
    else
    {
        InternalPluginFormat internalFormat;

        // Add the filter to the signal path.
        signalPath->addFilter(internalFormat.getDescriptionFor(InternalPluginFormat::audioInputFilter),
                              10.0f, 10.0f);

        // Add the associated PluginComponent.
        addFilter(signalPath->getNumFilters() - 1);
    }
}

void PluginField::enableMidiInput(bool val)
{
    int i;
    AudioProcessorGraph::Node::Ptr tempNode = nullptr;
    std::multimap<uint32, Mapping *>::iterator it;

    midiInputEnabled = val;

    if (!val)
    {
        // Delete mappings.
        for (it = mappings.begin(); it != mappings.end();)
        {
            MidiMapping *midiMapping = dynamic_cast<MidiMapping *>(it->second);

            if (midiMapping)
            {
                delete it->second;
                it = mappings.erase(it);
            }
            else
                ++it;
        }

        // Midi Input filter.
        {
            // Delete filter.
            for (i = (signalPath->getNumFilters() - 1); i >= 0; --i)
            {
                if (signalPath->getNode(i)->getProcessor()->getName() == "Midi Input")
                {
                    tempNode = signalPath->getNode(i);
                    deleteFilter(tempNode);
                }
            }

            // Delete PluginComponent.
            for (i = (getNumChildComponents() - 1); i >= 0; --i)
            {
                PluginComponent *comp = dynamic_cast<PluginComponent *>(getChildComponent(i));

                if (comp)
                {
                    if (comp->getNode() == tempNode)
                        delete removeChildComponent(i);
                }
            }
        }
        // Midi Interceptor filter.
        {
            // Delete filter.
            for (i = (signalPath->getNumFilters() - 1); i >= 0; --i)
            {
                if (signalPath->getNode(i)->getProcessor()->getName() == "Midi Interceptor")
                    deleteFilter(signalPath->getNode(i));
            }
        }
    }
    else
    {
        InternalPluginFormat internalFormat;

        // Add the filter to the signal path.
        signalPath->addFilter(internalFormat.getDescriptionFor(InternalPluginFormat::midiInputFilter),
                              10.0f, 120.0f);

        // Add the associated PluginComponent.
        addFilter(signalPath->getNumFilters() - 1);

        // Add the Midi Interceptor too.
        {
            MidiInterceptor p;
            juce::PluginDescription desc;

            p.fillInPluginDescription(desc);

            signalPath->addFilter(&desc, 100, 100);

            // And connect it up to the midi input.
            {
                AudioProcessorGraph::NodeID midiInput;
                AudioProcessorGraph::NodeID midiInterceptor;

                for (i = 0; i < signalPath->getNumFilters(); ++i)
                {
                    if (signalPath->getNode(i)->getProcessor()->getName() == "Midi Input")
                        midiInput = signalPath->getNode(i)->nodeID;
                    else if (signalPath->getNode(i)->getProcessor()->getName() == "Midi Interceptor")
                    {
                        midiInterceptor = signalPath->getNode(i)->nodeID;
                        dynamic_cast<MidiInterceptor *>(signalPath->getNode(i)->getProcessor())->setManager(&midiManager);
                    }
                }
                signalPath->addConnection(midiInput,
                                          AudioProcessorGraph::midiChannelIndex,
                                          midiInterceptor,
                                          AudioProcessorGraph::midiChannelIndex);
            }
        }
    }
}

void PluginField::enableOscInput(bool val)
{
    int i;
    std::multimap<uint32, Mapping *>::iterator it;
    AudioProcessorGraph::Node::Ptr tempNode = nullptr;

    oscInputEnabled = val;

    if (!val)
    {
        // Delete mappings.
        for (it = mappings.begin(); it != mappings.end();)
        {
            OscMapping *oscMapping = dynamic_cast<OscMapping *>(it->second);

            if (oscMapping)
            {
                delete it->second;
                it = mappings.erase(it);
            }
            else
                ++it;
        }

        // Delete filter.
        for (i = (signalPath->getNumFilters() - 1); i >= 0; --i)
        {
            if (signalPath->getNode(i)->getProcessor()->getName() == "OSC Input")
            {
                tempNode = signalPath->getNode(i);
                deleteFilter(tempNode);
            }
        }

        // Delete PluginComponent.
        for (i = (getNumChildComponents() - 1); i >= 0; --i)
        {
            PluginComponent *comp = dynamic_cast<PluginComponent *>(getChildComponent(i));

            if (comp)
            {
                if (comp->getNode() == tempNode)
                    delete removeChildComponent(i);
            }
        }
    }
    else
    {
        OscInput p;
        juce::PluginDescription desc;

        p.fillInPluginDescription(desc);

        signalPath->addFilter(&desc, 10, 215);

        addFilter(signalPath->getNumFilters() - 1);
    }
}

void PluginField::setAutoMappingsWindow(bool val)
{
    autoMappingsWindow = val;
}

void PluginField::setTempo(double val)
{
    tempo = val;
}

void PluginField::addFilter(int index, bool broadcastChangeMessage)
{
    int x, y;
    PluginComponent *plugin;
    AudioProcessorGraph::Node::Ptr node;

    if (index < signalPath->getNumFilters())
    {
        node = signalPath->getNode(index);

        if (node->getProcessor()->getName() != "Midi Interceptor")
        {
            // Make sure the plugin knows about the AudioPlayHead.
            node->getProcessor()->setPlayHead(this);

            plugin = new PluginComponent(node);
            x = signalPath->getNode(index)->properties.getWithDefault("x", 0);
            y = signalPath->getNode(index)->properties.getWithDefault("y", 0);
            plugin->setTopLeftPosition(x, y);
            plugin->addChangeListener(this);
            addAndMakeVisible(plugin);

            if (LogFile::getInstance().getIsLogging())
            {
                juce::String tempstr;

                tempstr << "Added plugin to signal path: " << node->getProcessor()->getName();
                LogFile::getInstance().logEvent("Pedalboard", tempstr);
            }

            // To make sure the plugin field bounds are correct.
            changeListenerCallback(plugin);

            if (broadcastChangeMessage)
                sendChangeMessage();
        }
    }
}

void PluginField::deleteFilter(AudioProcessorGraph::Node::Ptr node)
{
    int i;
    PluginConnection *connection;
    const AudioProcessorGraph::NodeID uid = node->nodeID;
    std::multimap<uint32, Mapping *>::iterator it;
    juce::String pluginName = node->getProcessor()->getName();

    // Disconnect any PluginConnections.
    for (i = getNumChildComponents() - 1; i >= 0; --i)
    {
        connection = dynamic_cast<PluginConnection *>(getChildComponent(i));

        if (connection)
        {
            const PluginPinComponent *src = connection->getSource();
            const PluginPinComponent *dest = connection->getDestination();
            AudioProcessorGraph::NodeID srcId, destId;

            // Had a crash here once, where dest was null. Not exactly sure
            // why that happened, but the following will at least prevent it
            // happening again.
            if (src)
                srcId = src->getUid();
            if (dest)
                destId = dest->getUid();

            if ((uid == srcId) || (uid == destId))
            {
                removeChildComponent(connection);
                delete connection;
            }
        }
    }

    // Delete any associated mappings.
    for (it = mappings.lower_bound(uid.uid);
         it != mappings.upper_bound(uid.uid);
         ++it)
    {
        delete it->second;
    }
    mappings.erase(uid.uid);

    // Unregister the filter from wanting MIDI over OSC.
    {
        BypassableInstance *proc = dynamic_cast<BypassableInstance *>(node->getProcessor());

        if (proc)
            oscManager.unregisterMIDIProcessor(proc);
    }

    signalPath->disconnectFilter(uid);
    signalPath->removeFilter(uid);

    if (LogFile::getInstance().getIsLogging())
    {
        juce::String tempstr;

        tempstr << "Deleted plugin from signal path: " << pluginName;
        LogFile::getInstance().logEvent("Pedalboard", tempstr);
    }

    sendChangeMessage();
}

void PluginField::updateProcessorName(uint32 id, const juce::String& val)
{
    userNames[id] = val;
}

void PluginField::addConnection(PluginPinComponent *source, bool connectAll)
{
    if (source)
    {
        auto* connection = new PluginConnection(source,
                                                nullptr,
                                                connectAll);

        connection->setSize(10, 12);
        addAndMakeVisible(connection);
        draggingConnection = connection;

        sendChangeMessage();
    }
}

void PluginField::dragConnection(int x, int y)
{
    if (draggingConnection)
    {
        juce::Component *c = getPinAt(x + 5, y);
        PluginPinComponent *p = dynamic_cast<PluginPinComponent *>(c);

        if (p)
        {
            if (p->getParameterPin() == draggingConnection->getParameterConnection())
            {
                juce::Point<int> tempPoint(p->getX() + 5, p->getY() + 6);

                tempPoint = getLocalPoint(p->getParentComponent(), tempPoint);
                draggingConnection->drag(tempPoint.getX(), tempPoint.getY());
            }
            else
                draggingConnection->drag(x, y);
        }
        else
            draggingConnection->drag(x, y);
    }
}

void PluginField::releaseConnection(int x, int y)
{
    if (draggingConnection)
    {
        juce::Component *c = getPinAt(x, y);
        PluginPinComponent *p = dynamic_cast<PluginPinComponent *>(c);

        repaint();

        if (p)
        {
            if (!p->getDirection())
            {
                const PluginPinComponent *s = draggingConnection->getSource();

                if ((s->getParameterPin() && p->getParameterPin()) ||
                   (!s->getParameterPin() && !p->getParameterPin()))
                {
                    signalPath->addConnection(s->getUid(),
                                              s->getChannel(),
                                              p->getUid(),
                                              p->getChannel());
                    draggingConnection->setDestination(p);

                    // If we should be connecting all the outputs and inputs
                    // of the two plugins (user holding down shift).
                    if (draggingConnection->getRepresentsAllOutputs())
                    {
                        connectAll(draggingConnection);
                        draggingConnection->setRepresentsAllOutputs(false);
                    }

                    if (p->getParameterPin())
                    {
                        PluginComponent *pComp = dynamic_cast<PluginComponent *>(p->getParentComponent());

                        if (pComp && autoMappingsWindow)
                            pComp->openMappingsWindow();
                    }
                    moveConnectionsBehind();

                    sendChangeMessage();
                }
                else
                {
                    removeChildComponent(draggingConnection);
                    delete draggingConnection;
                }
            }
        }
        else
        {
            removeChildComponent(draggingConnection);
            delete draggingConnection;
        }
        draggingConnection = nullptr;
    }
}

void PluginField::deleteConnection()
{
    int i;

    for (i = (getNumChildComponents() - 1); i >= 0; --i)
    {
        PluginConnection *c = dynamic_cast<PluginConnection *>(getChildComponent(i));

        if (c)
        {
            if (c->getSelected())
            {
                const PluginPinComponent *s = c->getSource();
                const PluginPinComponent *d = c->getDestination();

                signalPath->removeConnection(s->getUid(),
                                              s->getChannel(),
                                              d->getUid(),
                                              d->getChannel());
                removeChildComponent(c);
                delete c;

                // If it's a param connection, delete any MIDI or OSC mappings.
                if (c->getParameterConnection())
                {
                    AudioProcessorGraph::NodeID sourceId = s->getUid();
                    AudioProcessorGraph::NodeID destId = d->getUid();
                    juce::String tempstr = signalPath->getNodeForId(sourceId)->getProcessor()->getName();

                    // It's a Midi connection, so delete any associated Midi
                    // mappings for the destination plugin.
                    if (tempstr == "Midi Input")
                    {
                        std::multimap<uint32, Mapping *>::iterator it;

                        for (it = mappings.lower_bound(destId.uid);
                             it != mappings.upper_bound(destId.uid);)
                        {
                            MidiMapping *mapping = dynamic_cast<MidiMapping *>(it->second);

                            if (mapping)
                            {
                                delete mapping;
                                it = mappings.erase(it);
                            }
                            else
                                ++it;
                        }
                    }
                    else if (tempstr == "OSC Input")
                    {
                        std::multimap<uint32, Mapping *>::iterator it;

                        for (it = mappings.lower_bound(destId.uid);
                             it != mappings.upper_bound(destId.uid);)
                        {
                            OscMapping *mapping = dynamic_cast<OscMapping *>(it->second);

                            if (mapping)
                            {
                                delete it->second;
                                it = mappings.erase(it);
                            }
                            else
                                ++it;
                        }
                    }
                }
                sendChangeMessage();

                repaint();
            }
        }
    }
}

void PluginField::enableMidiForNode(AudioProcessorGraph::Node::Ptr node, bool val)
{
    int i;
    AudioProcessorGraph::Node::Ptr midiInput = nullptr;

    // Find the Midi Input node.
    for (i = 0; i < signalPath->getNumFilters(); ++i)
    {
        midiInput = signalPath->getNode(i);

        if (midiInput->getProcessor()->getName() == "Midi Input")
            break;
    }
    // Just in case.
    if (midiInput)
    {
        if (midiInput->getProcessor()->getName() != "Midi Input")
            return;
    }
    else
        return;

    // Check if there's a connection.
    bool connection = signalPath->connectionExists(midiInput->nodeID,
                                                    AudioProcessorGraph::midiChannelIndex,
                                                    node->nodeID,
                                                    AudioProcessorGraph::midiChannelIndex);
    if (val)
    {
        // If there's a connection, remove it.
        signalPath->removeConnection(midiInput->nodeID,
                                     AudioProcessorGraph::midiChannelIndex,
                                     node->nodeID,
                                     AudioProcessorGraph::midiChannelIndex);
    }
    else
    {
        // If there's not a connection, add it.
        signalPath->addConnection(midiInput->nodeID,
                                  AudioProcessorGraph::midiChannelIndex,
                                  node->nodeID,
                                  AudioProcessorGraph::midiChannelIndex);
    }
}

bool PluginField::getMidiEnabledForNode(AudioProcessorGraph::Node::Ptr node) const
{
    int i;
    AudioProcessorGraph::Node::Ptr midiInput = nullptr;

    // Find the Midi Input node.
    for (i = 0; i < signalPath->getNumFilters(); ++i)
    {
        midiInput = signalPath->getNode(i);

        if (midiInput->getProcessor()->getName() == "Midi Input")
            break;
        else
            midiInput = nullptr;
    }

    if (!midiInput)
        return false;
    else
        return signalPath->connectionExists(midiInput->nodeID,
                                             AudioProcessorGraph::midiChannelIndex,
                                             node->nodeID,
                                             AudioProcessorGraph::midiChannelIndex);
}

void PluginField::addMapping(Mapping *mapping)
{
    mappings.insert(std::make_pair(mapping->getPluginId(), mapping));
    sendChangeMessage();
}

void PluginField::removeMapping(Mapping *mapping)
{
    std::multimap<uint32, Mapping *>::iterator it;

    for (it = mappings.begin(); it != mappings.end(); ++it)
    {
        if (it->second == mapping)
        {
            delete it->second;
            mappings.erase(it);
            break;
        }
    }
    sendChangeMessage();
}

juce::Array<Mapping *> PluginField::getMappingsForPlugin(uint32 id)
{
    juce::Array<Mapping *> retval;
    std::multimap<uint32, Mapping *>::iterator it;

    for (it = mappings.lower_bound(id);
         it != mappings.upper_bound(id);
         ++it)
    {
        retval.add(it->second);
    }

    return retval;
}

void PluginField::socketDataArrived(char *data, int32 dataSize)
{
    // JUCE 8: raw OSC parsing via OSCInputStream is no longer available in
    // juce_osc. OSC messages are now received through juce::OSCReceiver
    // listener callbacks in MainPanel, which forwards them to OscMappingManager.
    // This method is kept for API compatibility but is a no-op.
    juce::ignoreUnused(data, dataSize);
}

std::unique_ptr<juce::XmlElement> PluginField::getXml() const
{
    int i;
    std::map<uint32, juce::String>::const_iterator it2;
    std::multimap<uint32, Mapping *>::const_iterator it;
    auto retval = std::make_unique<juce::XmlElement>("Patch");
    auto mappingsXml = std::make_unique<juce::XmlElement>("Mappings");
    auto userNamesXml = std::make_unique<juce::XmlElement>("UserNames");

    // Update saved window positions.
    for (i = 0; i < getNumChildComponents(); ++i)
    {
        PluginComponent *plugin = dynamic_cast<PluginComponent *>(getChildComponent(i));

        if (plugin)
            plugin->saveWindowState();
    }

    // Set the patch tempo.
    retval->setAttribute("tempo", tempo);

    // Add FilterGraph.
    retval->addChildElement(signalPath->createXml().release());

    // Add Mappings.
    for (it = mappings.begin(); it != mappings.end(); ++it)
        mappingsXml->addChildElement(it->second->getXml());
    retval->addChildElement(mappingsXml.release());

    // Add user names.
    for (it2 = userNames.begin(); it2 != userNames.end(); ++it2)
    {
        auto nameXml = std::make_unique<juce::XmlElement>("Name");

        nameXml->setAttribute("id", static_cast<int>(it2->first));
        nameXml->setAttribute("val", it2->second);

        userNamesXml->addChildElement(nameXml.release());
    }
    retval->addChildElement(userNamesXml.release());

    return retval;
}

void PluginField::loadFromXml(juce::XmlElement *patch)
{
    int i, j;
    juce::Array<uint32> paramConnections;

    // Delete all the filter and connection components.
    {
        // If we don't do this, the connections will try to contact their
        // pins, which may have already been deleted.
        for (i = (getNumChildComponents() - 1); i >= 0; --i)
        {
            PluginConnection *connection = dynamic_cast<PluginConnection *>(getChildComponent(i));

            if (connection)
            {
                removeChildComponent(connection);
                delete connection;
            }
        }
    }
    deleteAllChildren();
    repaint();

    // Wipe userNames.
    userNames.clear();

    // Clear and possibly load the signal path.
    clearMappings();
    if (patch)
    {
        tempo = patch->getDoubleAttribute("tempo", 120.0);

        signalPath->clear(false, false, false);
        if (auto* graphXml = patch->getChildByName("FILTERGRAPH"))
            signalPath->restoreFromXml(*graphXml);
    }
    else
        signalPath->clear(audioInputEnabled, midiInputEnabled);

    // Add the filter components.
    for (i = 0; i < signalPath->getNumFilters(); ++i)
        addFilter(i, false);

    // Update any plugin names.
    if (patch)
    {
        juce::XmlElement *userNamesXml = patch->getChildByName("UserNames");

        if (userNamesXml)
        {
            for (auto* e : userNamesXml->getChildIterator())
            {
                if (e->hasTagName("Name"))
                {
                    uint32 id = static_cast<uint32>(e->getIntAttribute("id"));
                    juce::String name = e->getStringAttribute("val");

                    for (i = 0; i < getNumChildComponents(); ++i)
                    {
                        PluginComponent *pluginComp = dynamic_cast<PluginComponent *>(getChildComponent(i));

                        if (pluginComp)
                        {
                            if (pluginComp->getNode()->nodeID.uid == id)
                            {
                                pluginComp->setUserName(name);
                                userNames[id] = name;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // Add the audio/midi connections.
    {
        struct NodeAndId
        {
            AudioProcessorGraph::Node::Ptr node;
            uint32 id;
        };
        juce::Array<NodeAndId> tempNodes;

        // Stick all the nodes and their ids in an array.
        for (i = 0; i < signalPath->getNumFilters(); ++i)
        {
            NodeAndId n;

            n.node = signalPath->getNode(i);
            n.id = n.node->nodeID.uid;
            tempNodes.add(n);
        }

        // Add the audio/midi connections.
        auto connections = signalPath->getConnections();
        for (const auto& connection : connections)
        {
            AudioProcessorGraph::Node::Ptr sourceNode = nullptr;
            AudioProcessorGraph::Node::Ptr destNode = nullptr;
            PluginComponent *sourceComp = nullptr;
            PluginComponent *destComp = nullptr;
            PluginPinComponent *sourcePin;
            PluginPinComponent *destPin;

            // Fill out sourceNode and destNode.
            for (j = 0; j < tempNodes.size(); ++j)
            {
                if (tempNodes[j].id == connection.source.nodeID.uid)
                    sourceNode = tempNodes[j].node;
                else if (tempNodes[j].id == connection.destination.nodeID.uid)
                    destNode = tempNodes[j].node;
            }

            if (!sourceNode || !destNode)
            {
                jassertfalse;
                continue;
            }
            else if (destNode->getProcessor()->getName() == "Midi Interceptor")
                continue;

            // Now get the source and destination components.
            for (j = 0; j < getNumChildComponents(); ++j)
            {
                PluginComponent *pluginComp = dynamic_cast<PluginComponent *>(getChildComponent(j));

                if (pluginComp)
                {
                    if (pluginComp->getNode() == sourceNode)
                        sourceComp = pluginComp;
                    else if (pluginComp->getNode() == destNode)
                        destComp = pluginComp;
                }
            }

            if (!sourceComp || !destComp)
            {
                jassertfalse;
                continue;
            }

            if ((connection.source.channelIndex == connection.destination.channelIndex) &&
               (connection.source.channelIndex == AudioProcessorGraph::midiChannelIndex))
            {
                sourcePin = sourceComp->getParamPin(0);
                destPin = destComp->getParamPin(0);

                paramConnections.add(connection.destination.nodeID.uid);
            }
            else
            {
                sourcePin = sourceComp->getOutputPin(connection.source.channelIndex);
                destPin = destComp->getInputPin(connection.destination.channelIndex);
            }

            if (!sourcePin || !destPin)
            {
                jassertfalse;
                continue;
            }

            addAndMakeVisible(new PluginConnection(sourcePin, destPin));
        }
    }

    // Add the mappings.
    if (patch)
    {
        juce::XmlElement *mappingsXml = patch->getChildByName("Mappings");

        if (mappingsXml)
        {
            for (auto* e : mappingsXml->getChildIterator())
            {
                if (e->hasTagName("MidiMapping"))
                {
                    MidiMapping *mapping = new MidiMapping(&midiManager,
                                                           signalPath,
                                                           e);
                    midiManager.registerMapping(mapping->getCc(), mapping);

                    mappings.insert(std::make_pair(mapping->getPluginId(), mapping));
                }
                else if (e->hasTagName("OscMapping"))
                {
                    OscMapping *mapping = new OscMapping(&oscManager,
                                                         signalPath,
                                                         e);
                    oscManager.registerMapping(mapping->getAddress(), mapping);

                    mappings.insert(std::make_pair(mapping->getPluginId(), mapping));
                }
            }
        }
    }

    // Connect the Midi Interceptor to the MidiMappingManager.
    if (midiInputEnabled)
    {
        MidiInterceptor *interceptor = nullptr;

        for (i = 0; i < signalPath->getNumFilters(); ++i)
        {
            interceptor = dynamic_cast<MidiInterceptor *>(signalPath->getNode(i)->getProcessor());

            if (interceptor)
            {
                interceptor->setManager(&midiManager);
                break;
            }
        }
    }

    // Add in any parameter mapping connections.
    {
        std::multimap<uint32, Mapping *>::iterator it;
        PluginPinComponent *midiInput = nullptr;
        PluginPinComponent *oscInput = nullptr;

        // Get the Midi Input and OSC Input pins.
        for (i = 0; i < getNumChildComponents(); ++i)
        {
            PluginComponent *comp = dynamic_cast<PluginComponent *>(getChildComponent(i));

            if (comp)
            {
                juce::String tempstr = comp->getNode()->getProcessor()->getName();

                if (tempstr == "Midi Input")
                    midiInput = comp->getParamPin(0);
                else if (tempstr == "OSC Input")
                    oscInput = comp->getParamPin(0);
            }
        }

        if (midiInputEnabled && midiInput)
        {
            // Add a connection for each Midi mapping, checking that we
            // don't already have an identical one.
            for (it = mappings.begin(); it != mappings.end(); ++it)
            {
                MidiMapping *midiMapping = dynamic_cast<MidiMapping *>(it->second);

                if (midiMapping)
                {
                    uint32 uid = midiMapping->getPluginId();

                    if (!paramConnections.contains(uid))
                    {
                        // Find the PluginComponent matching this uid.
                        for (i = 0; i < getNumChildComponents(); ++i)
                        {
                            PluginComponent *comp = dynamic_cast<PluginComponent *>(getChildComponent(i));

                            if (comp)
                            {
                                if (comp->getNode()->nodeID.uid == uid)
                                {
                                    PluginPinComponent *paramInput = nullptr;

                                    for (j = 0; j < comp->getNumParamPins(); ++j)
                                    {
                                        if (!comp->getParamPin(j)->getDirection())
                                        {
                                            paramInput = comp->getParamPin(j);
                                            break;
                                        }
                                    }

                                    jassert(paramInput);

                                    addAndMakeVisible(new PluginConnection(midiInput,
                                                                           paramInput));
                                    paramConnections.add(uid);

                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (oscInputEnabled && oscInput)
        {
            // Ditto for the osc mappings.
            paramConnections.clear();
            for (it = mappings.begin(); it != mappings.end(); ++it)
            {
                OscMapping *oscMapping = dynamic_cast<OscMapping *>(it->second);

                if (oscMapping)
                {
                    uint32 uid = oscMapping->getPluginId();

                    if (!paramConnections.contains(uid))
                    {
                        // Find the PluginComponent matching this uid.
                        for (i = 0; i < getNumChildComponents(); ++i)
                        {
                            PluginComponent *comp = dynamic_cast<PluginComponent *>(getChildComponent(i));

                            if (comp)
                            {
                                if (comp->getNode()->nodeID.uid == uid)
                                {
                                    PluginPinComponent *paramInput = nullptr;

                                    for (j = 0; j < comp->getNumParamPins(); ++j)
                                    {
                                        if (!comp->getParamPin(j)->getDirection())
                                        {
                                            paramInput = comp->getParamPin(j);
                                            break;
                                        }
                                    }

                                    jassert(paramInput);

                                    addAndMakeVisible(new PluginConnection(oscInput,
                                                                           paramInput));
                                    paramConnections.add(uid);

                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Make sure any disabled inputs/outputs don't get accidentally re-enabled.
    if (!audioInputEnabled)
        enableAudioInput(audioInputEnabled);
    if (!midiInputEnabled)
        enableMidiInput(midiInputEnabled);
    if (!oscInputEnabled)
        enableOscInput(oscInputEnabled);

    moveConnectionsBehind();
    repaint();
}

void PluginField::clear()
{
    int i;

    // Delete all the filter and connection components.
    {
        // If we don't do this, the connections will try to contact their
        // pins, which may have already been deleted.
        for (i = (getNumChildComponents() - 1); i >= 0; --i)
        {
            PluginConnection *connection = dynamic_cast<PluginConnection *>(getChildComponent(i));

            if (connection)
            {
                removeChildComponent(connection);
                delete connection;
            }
        }
    }
    deleteAllChildren();
    repaint();

    // Wipe userNames.
    userNames.clear();

    // Clear any mappings.
    clearMappings();

    // Clear the signal path.
    signalPath->clear(audioInputEnabled, midiInputEnabled);

    // Add OSC input.
    if (oscInputEnabled)
    {
        OscInput p;
        juce::PluginDescription desc;

        p.fillInPluginDescription(desc);

        signalPath->addFilter(&desc, 10, 215);
    }

    // Setup gui.
    for (i = 0; i < signalPath->getNumFilters(); ++i)
        addFilter(i);

    // Add MidiInterceptor.
    if (midiInputEnabled)
    {
        MidiInterceptor p;
        juce::PluginDescription desc;

        p.fillInPluginDescription(desc);

        signalPath->addFilter(&desc, 100, 100);

        // And connect it up to the midi input.
        {
            AudioProcessorGraph::NodeID midiInput;
            AudioProcessorGraph::NodeID midiInterceptor;

            for (i = 0; i < signalPath->getNumFilters(); ++i)
            {
                if (signalPath->getNode(i)->getProcessor()->getName() == "Midi Input")
                    midiInput = signalPath->getNode(i)->nodeID;
                else if (signalPath->getNode(i)->getProcessor()->getName() == "Midi Interceptor")
                {
                    midiInterceptor = signalPath->getNode(i)->nodeID;
                    dynamic_cast<MidiInterceptor *>(signalPath->getNode(i)->getProcessor())->setManager(&midiManager);
                }
            }
            signalPath->addConnection(midiInput,
                                      AudioProcessorGraph::midiChannelIndex,
                                      midiInterceptor,
                                      AudioProcessorGraph::midiChannelIndex);
        }
    }

    repaint();
}

void PluginField::clearDoubleClickMessage()
{
    displayDoubleClickMessage = false;
    repaint();
}

void PluginField::clearMappings()
{
    std::multimap<uint32, Mapping *>::iterator it;

    for (it = mappings.begin(); it != mappings.end(); ++it)
        delete it->second;

    mappings.clear();
}

void PluginField::handleOscBundle(const juce::OSCBundle& bundle)
{
    for (int i = 0; i < bundle.size(); ++i)
    {
        const auto& elem = bundle[i];

        if (elem.isBundle())
            handleOscBundle(elem.getBundle());
        else if (elem.isMessage())
            oscManager.messageReceived(elem.getMessage());
    }
}

void PluginField::moveConnectionsBehind()
{
    int i;

    for (i = (getNumChildComponents() - 1); i >= 0; --i)
    {
        PluginConnection *connection = dynamic_cast<PluginConnection *>(getChildComponent(i));

        if (connection)
            connection->toBack();
        else
            getChildComponent(i)->toFront(false);
    }
}

juce::Component *PluginField::getPinAt(const int x, const int y)
{
    juce::Point<int> pos(x, y);

    if (isVisible() &&
        static_cast<unsigned int>(x) < static_cast<unsigned int>(getWidth()) &&
        static_cast<unsigned int>(y) < static_cast<unsigned int>(getHeight()) &&
        hitTest(x, y))
    {
        for (int i = getNumChildComponents(); --i >= 0;)
        {
            juce::Component* const child = getChildComponent(i);

            if (!dynamic_cast<PluginConnection *>(child))
            {
                juce::Rectangle<int> updatedRect(child->getX() - 16,
                                                 child->getY() - 16,
                                                 child->getWidth() + 32,
                                                 child->getHeight() + 32);

                if (updatedRect.contains(pos))
                {
                    if (pos.getX() < child->getX())
                        pos.addXY((child->getX() - pos.getX()), 0);
                    if (pos.getY() < child->getY())
                        pos.addXY(0, (child->getY() - pos.getY()));

                    if (pos.getX() > (child->getX() + child->getWidth()))
                        pos.addXY(-(pos.getX() - (child->getX() + child->getWidth())), 0);
                    if (pos.getY() > (child->getY() + child->getHeight()))
                        pos.addXY(0, -(pos.getY() - (child->getY() + child->getHeight())));

                    juce::Component* const c = child->getComponentAt(pos.getX() - child->getX(),
                                                                     pos.getY() - child->getY());

                    if (c != nullptr)
                        return c;
                }
            }
        }

        return this;
    }

    return nullptr;
}

void PluginField::connectAll(PluginConnection *connection)
{
    PluginComponent *source = dynamic_cast<PluginComponent *>(connection->getSource()->getParentComponent());
    PluginComponent *dest = dynamic_cast<PluginComponent *>(connection->getDestination()->getParentComponent());

    if (source && dest)
    {
        int left, right;
        int numSources = source->getNumOutputPins();
        int numDests = dest->getNumInputPins();
        PluginPinComponent *sourcePin;
        PluginPinComponent *destPin;

        for (left = 0; left < numSources; ++left)
        {
            if (source->getOutputPin(left) == connection->getSource())
            {
                ++left;
                break;
            }
        }
        for (right = 0; right < numDests; ++right)
        {
            if (dest->getInputPin(right) == connection->getDestination())
            {
                ++right;
                break;
            }
        }

        for (; (left < numSources) && (right < numDests); ++left, ++right)
        {
            sourcePin = source->getOutputPin(left);
            destPin = dest->getInputPin(right);
            signalPath->addConnection(sourcePin->getUid(),
                                      sourcePin->getChannel(),
                                      destPin->getUid(),
                                      destPin->getChannel());
            addAndMakeVisible(new PluginConnection(sourcePin, destPin));
        }
    }
}
