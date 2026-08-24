// MainTransport.h - Singleton representing the app's main playback transport.
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

#ifndef MAINTRANSPORT_H_
#define MAINTRANSPORT_H_

#include <JuceHeader.h>

/// Singleton representing the app's main playback transport.
///
/// Uses the ChangeBroadcaster mechanism to inform any interested transports
/// about changes in the main transport's state. When the user clicks the
/// main transport play button, change messages are sent to registered
/// ChangeListeners, which then query MainTransport to determine whether they
/// should start or stop playing, and whether they should return to zero.
/// This happens asynchronously because the signal may come from a MIDI source
/// running in the audio thread.
class MainTransport : public ChangeBroadcaster
{
  public:
    /// Destructor.
    ~MainTransport();

    /// Registers a transport with this singleton.
    void registerTransport(ChangeListener* transport);
    /// Unregisters a transport from this singleton.
    void unregisterTransport(ChangeListener* transport);

    /// Called from each transport when they have reached the end of their timeline.
    void transportFinished();

    /// Called by MainPanel's play button to toggle the current state.
    void toggleState();
    /// Called by MainPanel's rtz button to return all transports to start.
    void setReturnToZero();

    /// Returns the transport's current state (true = playing).
    bool getState() const { return state; }

    /// Returns true if the calling transport needs to return to its start position.
    ///
    /// Uses rtzCount to ensure returnToZero is set to false after all
    /// transports have been informed about the change.
    bool getReturnToZero();

    juce_DeclareSingleton(MainTransport, true)

  private:
    MainTransport();

    /// Registered transports (needed because ChangeBroadcaster cannot be queried for listener count).
    Array<ChangeListener*> transports;

    bool state;
    int returnToZero;
    int transportsPlaying;
};

#endif
