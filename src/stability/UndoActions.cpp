// UndoActions.cpp - Undoable actions for plugin and connection operations.
//
// This file is part of Pedalboard3, an audio plugin host.
// Ported from the Pedalboard3-VST3 fork by Project12x.
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

#include "UndoActions.h"

#include "FilterGraph.h"

#include <spdlog/spdlog.h>

// Helper: add a plugin and retrieve the resulting NodeID.
//
// The target FilterGraph::addFilter returns void, so we capture the node
// count before and after the call to identify the newly added node.
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

// AddPluginAction

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

// RemovePluginAction

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

// AddConnectionAction

bool AddConnectionAction::perform()
{
    return filterGraph.addConnection(sourceNode, sourceChannel, destNode, destChannel);
}

bool AddConnectionAction::undo()
{
    filterGraph.removeConnection(sourceNode, sourceChannel, destNode, destChannel);
    return true;
}

// RemoveConnectionAction

bool RemoveConnectionAction::perform()
{
    filterGraph.removeConnection(sourceNode, sourceChannel, destNode, destChannel);
    return true;
}

bool RemoveConnectionAction::undo()
{
    return filterGraph.addConnection(sourceNode, sourceChannel, destNode, destChannel);
}
