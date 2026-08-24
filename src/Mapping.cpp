// Mapping.cpp - Base class for parameter mappings.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
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

#include "Mapping.h"
#include "FilterGraph.h"
#include "BypassableInstance.h"

//------------------------------------------------------------------------------
Mapping::Mapping(FilterGraph* graph, uint32 pluginId, int param)
    : filterGraph(graph)
    , plugin(pluginId)
    , parameter(param)
{
}

//------------------------------------------------------------------------------
Mapping::Mapping(FilterGraph* graph, XmlElement* e)
    : filterGraph(graph)
{
    if (e)
    {
        plugin = static_cast<uint32>(e->getIntAttribute("pluginId"));
        parameter = e->getIntAttribute("parameter");
    }
}

//------------------------------------------------------------------------------
Mapping::~Mapping() = default;

//------------------------------------------------------------------------------
void Mapping::updateParameter(float val)
{
    auto node = filterGraph->getNodeForId(AudioProcessorGraph::NodeID(plugin));
    if (node == nullptr)
        return;

    AudioProcessor* filter = node->getProcessor();

    if (parameter == -1)
    {
        auto* bypassable = dynamic_cast<BypassableInstance*>(filter);
        if (bypassable)
            bypassable->setBypass(val > 0.5f);
    }
    else
    {
        // JUCE 8: use AudioProcessorParameter array instead of deprecated setParameter()
        auto* pluginInstance = dynamic_cast<AudioPluginInstance*>(filter);
        if (pluginInstance != nullptr)
        {
            auto& params = pluginInstance->getParameters();
            if (parameter >= 0 && parameter < params.size())
                params[parameter]->setValue(val);
        }
    }
}

//------------------------------------------------------------------------------
void Mapping::setParameter(int val)
{
    parameter = val;
}
