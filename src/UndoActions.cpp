/*
  ==============================================================================

    UndoActions.cpp
    Part of Pedalboard3

    Implementation of undoable actions for plugin and connection operations.

    Adapted for the target FilterGraph API which does not provide raw
    (non-undoable) addFilter/removeFilter/addConnection/removeConnection
    methods. The regular methods are used instead. Since addFilter returns
    void (not NodeID), the node ID is retrieved after adding by inspecting
    the last node in the graph.

  ==============================================================================
*/

#include "UndoActions.h"

#include "FilterGraph.h"

#include <spdlog/spdlog.h>

//==============================================================================
// Helper: add a plugin and retrieve the resulting NodeID.
//
// The target FilterGraph::addFilter returns void, so we capture the node
// count before and after the call to identify the newly added node.
//==============================================================================
static juce::AudioProcessorGraph::NodeID addFilterAndGetNodeId(FilterGraph& graph,
                                                               const juce::PluginDescription* desc, double x,
                                                               double y)
{
    int countBefore = graph.getNumFilters();
    graph.addFilter(desc, x, y);
    int countAfter = graph.getNumFilters();

    if (countAfter <= countBefore)
    {
        spdlog::error("[UndoActions] addFilter did not increase node count (before={}, after={})", countBefore,
                      countAfter);
        return juce::AudioProcessorGraph::NodeID();
    }

    auto node = graph.getNode(countAfter - 1);
    if (node)
        return node->nodeID;

    spdlog::error("[UndoActions] Failed to get node at index {}", countAfter - 1);
    return juce::AudioProcessorGraph::NodeID();
}

//==============================================================================
// AddPluginAction
//==============================================================================

bool AddPluginAction::perform()
{
    spdlog::debug("[AddPluginAction::perform] About to call addFilter for: {}",
                  pluginDescription.name.toStdString());
    spdlog::default_logger()->flush();

    // Add the plugin using the regular method and retrieve the node ID
    nodeId = addFilterAndGetNodeId(filterGraph, &pluginDescription, x, y);

    spdlog::debug("[AddPluginAction::perform] addFilter returned nodeId: {}", nodeId.uid);
    spdlog::default_logger()->flush();

    bool result = nodeId != juce::AudioProcessorGraph::NodeID();

    spdlog::debug("[AddPluginAction::perform] Returning: {}", result);
    spdlog::default_logger()->flush();

    return result;
}

bool AddPluginAction::undo()
{
    if (nodeId != juce::AudioProcessorGraph::NodeID())
    {
        filterGraph.removeFilter(nodeId);
        return true;
    }
    return false;
}

//==============================================================================
// RemovePluginAction
//==============================================================================

bool RemovePluginAction::perform()
{
    filterGraph.removeFilter(nodeId);
    return true;
}

bool RemovePluginAction::undo()
{
    // Recreate the plugin
    auto newId = addFilterAndGetNodeId(filterGraph, &pluginDescription, x, y);

    if (newId != juce::AudioProcessorGraph::NodeID())
    {
        // Note: The node ID may be different after recreation.
        // Update our stored nodeId to the new one for future operations.
        nodeId = newId;

        // Restore all connections that involved this node
        for (const auto& conn : connections)
        {
            // Update connection references to use new node ID
            auto srcNode = (conn.source.nodeID == nodeId) ? newId : conn.source.nodeID;
            auto destNode = (conn.destination.nodeID == nodeId) ? newId : conn.destination.nodeID;

            filterGraph.addConnection(srcNode, conn.source.channelIndex, destNode, conn.destination.channelIndex);
        }
        return true;
    }
    return false;
}

//==============================================================================
// AddConnectionAction
//==============================================================================

bool AddConnectionAction::perform()
{
    return filterGraph.addConnection(sourceNode, sourceChannel, destNode, destChannel);
}

bool AddConnectionAction::undo()
{
    filterGraph.removeConnection(sourceNode, sourceChannel, destNode, destChannel);
    return true;
}

//==============================================================================
// RemoveConnectionAction
//==============================================================================

bool RemoveConnectionAction::perform()
{
    filterGraph.removeConnection(sourceNode, sourceChannel, destNode, destChannel);
    return true;
}

bool RemoveConnectionAction::undo()
{
    return filterGraph.addConnection(sourceNode, sourceChannel, destNode, destChannel);
}
