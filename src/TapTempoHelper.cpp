// TapTempoHelper.cpp - Calculates tempo from taps.
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

#include "TapTempoHelper.h"

#include <cmath>

//------------------------------------------------------------------------------
TapTempoHelper::TapTempoHelper()
{
    for (int i = 0; i < NumValues; ++i)
        times[i] = 0.0;
}

//------------------------------------------------------------------------------
TapTempoHelper::~TapTempoHelper() = default;

//------------------------------------------------------------------------------
double TapTempoHelper::updateTempo(double seconds)
{
    double tempd;
    double delta = 0.0;
    int numCounted = 0;
    double retval = 0.0;
    bool blankTimes = false;
    const double lowerLimit = (60.0 / 30.0); // 30 BPM => 2 seconds between taps

    // Check if we need to blank the times.
    for (int i = 0; i < NumValues; ++i)
    {
        if (times[i] <= 0.0)
            blankTimes = true;
        else if (std::fabs(times[i] - seconds) > lowerLimit)
            blankTimes = true;
    }

    // Blank the times if necessary.
    if (blankTimes)
    {
        for (int i = 0; i < NumValues; ++i)
            times[i] = seconds;
    }
    else
    {
        // Update the stored values (shift right).
        for (int i = (NumValues - 1); i > 0; --i)
            times[i] = times[i - 1];
        times[0] = seconds;
    }

    // Add up the differences between stored values.
    for (int i = 1; i < NumValues; ++i)
    {
        tempd = std::fabs(times[i] - times[i - 1]);

        if (tempd > 0.0)
        {
            delta += tempd;
            ++numCounted;
        }
    }

    // Take the average as the tempo.
    if (numCounted > 0)
    {
        retval = delta / static_cast<double>(numCounted);
        retval = (1.0 / retval) * 60.0; // BPM calculation
    }

    return retval;
}
