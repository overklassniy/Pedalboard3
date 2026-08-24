// Mapping.h - Base class for parameter mappings.
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

#ifndef MAPPING_H_
#define MAPPING_H_

#include <JuceHeader.h>

class FilterGraph;

/// The base class for all parameter mappings.
class Mapping
{
  public:
    Mapping(FilterGraph* graph, uint32 pluginId, int param);
    /// Constructor to load Mapping parameters from an XmlElement.
    Mapping(FilterGraph* graph, XmlElement* e);
    virtual ~Mapping();

    /// Returns an XmlElement representing this Mapping.
    virtual XmlElement* getXml() const = 0;

    /// Returns the id of the plugin this mapping applies to.
    uint32 getPluginId() const { return plugin; }
    /// Returns the index of the plugin parameter this mapping applies to.
    int getParameter() const { return parameter; }

    /// Sets this mapping's parameter index.
    void setParameter(int val);

  protected:
    /// Called from subclasses to update their parameter.
    void updateParameter(float val);

  private:
    FilterGraph* filterGraph;
    uint32 plugin;
    int parameter;
};

#endif
