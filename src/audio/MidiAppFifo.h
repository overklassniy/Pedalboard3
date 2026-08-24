// MidiAppFifo.h - Lock-free FIFO for audio-to-message-thread communication.
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

#ifndef MIDIAPPFIFO_H_
#define MIDIAPPFIFO_H_

#include <JuceHeader.h>

class FilterGraph;

/// Lock-free FIFO used to pass messages from the audio thread to the message thread.
class MidiAppFifo {
  public:
    /// A deferred parameter change queued from the audio thread.
    struct PendingParamChange {
        FilterGraph* graph;
        uint32 pluginId;
        /// Parameter index to change, or -1 for bypass.
        int paramIndex;
        float value;
    };

    /// Constructor. Initializes all FIFOs and zero-fills the buffers.
    MidiAppFifo();
    ~MidiAppFifo();

    /// Writes a CommandID to the FIFO.
    ///
    /// @param id The CommandID to write.
    void writeID(CommandID id);
    /// Reads a CommandID from the FIFO.
    ///
    /// @return The next CommandID, or static_cast<CommandID>(-1) if the FIFO is empty.
    CommandID readID();
    /// Returns the number of IDs waiting in the FIFO.
    int getNumWaitingID() const { return idFifo.getNumReady(); }

    /// Writes a new tempo to the FIFO.
    ///
    /// @param tempo The tempo value to write.
    void writeTempo(double tempo);
    /// Reads a tempo from the FIFO.
    ///
    /// @return The next tempo value, or 120.0 if the FIFO is empty.
    double readTempo();
    /// Returns the number of tempos waiting in the FIFO.
    int getNumWaitingTempo() const { return tempoFifo.getNumReady(); }

    /// Writes a patch change to the FIFO.
    ///
    /// @param index The patch index to write.
    void writePatchChange(int index);
    /// Reads a patch change from the FIFO.
    ///
    /// @return The next patch index, or 0 if the FIFO is empty.
    int readPatchChange();
    /// Returns the number of patch changes waiting in the FIFO.
    int getNumWaitingPatchChange() const { return patchChangeFifo.getNumReady(); }

    /// Writes a deferred parameter change to the FIFO (audio thread).
    ///
    /// @param graph The FilterGraph the parameter change applies to.
    /// @param pluginId The UID of the plugin whose parameter to change.
    /// @param paramIndex The parameter index to change, or -1 for bypass.
    /// @param value The new normalized parameter value (0-1).
    void writeParamChange(FilterGraph* graph, uint32 pluginId, int paramIndex, float value);
    /// Reads a deferred parameter change from the FIFO (message thread).
    ///
    /// @param out Output parameter filled with the next pending change.
    /// @return True if a change was read, false if the FIFO was empty.
    bool readParamChange(PendingParamChange& out);
    /// Returns the number of parameter changes waiting in the FIFO.
    int getNumWaitingParamChange() const { return paramChangeFifo.getNumReady(); }

  private:
    /// Number of slots in each FIFO buffer.
    enum { BufferSize = 1024 };

    /// Protects all write operations for multi-producer safety.
    /// AbstractFifo is SPSC; multiple threads (MIDI audio + OSC network) may
    /// write concurrently, so we serialise the producer side with a SpinLock.
    /// SpinLock is RT-safe (busy-wait, no OS blocking).
    juce::SpinLock writeLock;

    /// FIFO and backing buffer for CommandID messages.
    AbstractFifo idFifo;
    CommandID idBuffer[BufferSize];

    /// FIFO and backing buffer for tempo changes.
    AbstractFifo tempoFifo;
    double tempoBuffer[BufferSize];

    /// FIFO and backing buffer for patch change indices.
    AbstractFifo patchChangeFifo;
    int patchChangeBuffer[BufferSize];

    /// FIFO and backing buffer for deferred parameter changes.
    AbstractFifo paramChangeFifo;
    PendingParamChange paramChangeBuffer[BufferSize];
};

#endif
