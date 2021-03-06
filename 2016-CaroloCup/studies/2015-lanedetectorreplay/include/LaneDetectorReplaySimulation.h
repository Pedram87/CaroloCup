/**
 * 2015-example-lanedetector - This test suite demonstrates how to
 *                             compose a simulation using recordings.
 * Copyright (C) 2012 - 2015 Christian Berger
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANEDETECTORSIMULATION_H_
#define LANEDETECTORSIMULATION_H_

#include "opendavinci/odcore/opendavinci.h"

namespace simulation {
    class LaneDetectorSimulation {
        public:
            /**
             * This method executes the simulation and returns true
             * if everything ran as expected.
             *
             * @return true if everything ran as expected.
             */
            bool run();
    };
}

#endif // LANEDETECTORSIMULATION_H_
