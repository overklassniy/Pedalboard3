// PedalboardProcessor.h - Abstract base class for internal processors.
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

#ifndef PEDALBOARDPROCESSOR_H_
#define PEDALBOARDPROCESSOR_H_

#include <JuceHeader.h>

#include <stdint.h>

/// Abstract base of all the internal processors.
class PedalboardProcessor : public AudioPluginInstance
{
  public:
    ~PedalboardProcessor() override = default;

    /// Returns the component which is added to the instance's PluginComponent.
    ///
    /// Will be deleted by the caller.
    virtual Component* getControls() = 0;
    /// Returns the size of the controls component.
    virtual Point<int> getSize() = 0;
};

#endif
