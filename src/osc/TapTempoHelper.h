// TapTempoHelper.h - Calculates tempo from taps.
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

#ifndef TAPTEMPOHELPER_H_
#define TAPTEMPOHELPER_H_

/// Calculates tempo from tap intervals.
///
/// Stores 4 values and uses the average of the differences to calculate
/// tempo. Each value represents seconds since an arbitrary start point.
/// If the calculated tempo is less than 30 BPM, the values are reset.
class TapTempoHelper
{
  public:
    TapTempoHelper();
    ~TapTempoHelper();

    /// Updates the tempo and returns the new value.
    ///
    /// @param seconds  Seconds since app start or arbitrary start point,
    ///                  representing the time at which the user tapped.
    /// @return The new tempo in BPM. May be 0 if not enough data yet.
    double updateTempo(double seconds);

  private:
    enum
    {
        NumValues = 4
    };

    double times[NumValues];
};

#endif
