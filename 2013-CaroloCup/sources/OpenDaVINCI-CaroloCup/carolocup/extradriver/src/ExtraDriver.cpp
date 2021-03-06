/*
 * CaroloCup.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#include "core/io/ContainerConference.h"
#include "core/data/Container.h"
#include "core/data/TimeStamp.h"
#include "core/data/control/ForceControl.h"
#include "core/data/environment/VehicleData.h"
#include "core/base/LIFOQueue.h"
#include "SensorBoardData.h"
#include "ExtraDriver.h"
#include "LaneDetectionData.h"
#include "SensorDetectionData.h"

#include <stdio.h>
#include <math.h>

namespace carolocup {

using namespace std;
using namespace core::base;
using namespace core::data;
using namespace core::data::control;
using namespace core::data::environment;

double force;
double old_y = 0;
double old_degree = 0;
double aver[3] = { 0, 0, 0 };
int index = 0;
int counter = 0;
int temp = 0;

ExtraDriver::ExtraDriver(const int32_t &argc, char **argv) :
	ConferenceClientModule(argc, argv, "ExtraDriver") {
}

ExtraDriver::~ExtraDriver() {
}

void ExtraDriver::setUp() {
	// This method will be call automatically _before_ running body().
}

void ExtraDriver::tearDown() {
	// This method will be call automatically _after_ return from body().
}

// This method will do the main data processing job.
ModuleState::MODULE_EXITCODE ExtraDriver::body() {

	core::base::LIFOQueue lifo;
	addDataStoreFor(lifo);

	const double MAXIMUM_SPEED = 0.75;
	double w = 0;
	double e = 0;
	double esum = 0;
	double esum_str = 0;
	double x = 0;
	double Kp = 1.0;
	double Ki = 1.0;
	TimeStamp previousTime_str;
	TimeStamp previousTime_brk;

	double angle = 0;
	double sensor;
	double parking;

	enum EXTRADRIVER_STATES {
		RUNNING_STATE = 0, INTERSECTION_STATE = 1
	} state;
	state = RUNNING_STATE;

	while (getModuleState() == ModuleState::RUNNING) {
		bool hasReceivedLaneDetectionData = false;
		bool hasReceivedSensorDetectionData = false;

		LaneDetectionData theData;

		SensorDetectionData theSensorDistance;

		while (!lifo.isEmpty()) {
			// Read the recently received container.
			Container con = lifo.pop();

			// Check the container type if it is one of our expected types.
			if (con.getDataType() == Container::USER_DATA_1) {
				// We have found our expected container.
				theData = con.getData<LaneDetectionData> ();
				hasReceivedLaneDetectionData = true;
				break;
			}
			// Check the container type if it is one of our expected types.
			if (con.getDataType() == Container::USER_DATA_2) {
				// We have found our expected container.
				theSensorDistance = con.getData<SensorDetectionData> ();
				hasReceivedSensorDetectionData = true;
				break;
			}

		}
		// Clear the remaining data.
		lifo.clear();

		if (hasReceivedLaneDetectionData) {
			// Do something with your average attribute:
			angle = theData.getNumericalValue();

			// Do something with your average attribute:
			parking = theData.getDistance();

			cerr << "Received data from lanedetector: angle=" << angle
					<< ", parking=" << parking << endl;
		}

		if (hasReceivedSensorDetectionData) {
			// Do something with your average attribute:
			sensor = theSensorDistance.getNumericalValue();

			cerr << " Value from sensorboard "
					<< theSensorDistance.getNumericalValue() << endl;
		}

		// Get most recent vehicle data:
		Container containerVehicleData = getKeyValueDataStore().get(
				Container::VEHICLEDATA);
		VehicleData vd = containerVehicleData.getData<VehicleData> ();
		cerr << "Most recent vehicle data: '" << vd.toString() << "'" << endl;

		TimeStamp currentTime_str;
		double timeStep_str = (currentTime_str.toMicroseconds()
				- previousTime_str.toMicroseconds()) / (1000.0 * 1000.0);

		previousTime_str = currentTime_str;

		// Create forcecontrol data.
		ForceControl fc;

		if (angle > 361) {
			state = INTERSECTION_STATE;
		}

		switch (state) {

		case RUNNING_STATE: {
			// Angle adjustment
			double degree = angle - 90;
			cerr << "degree = " << degree << endl;

			//Apply forces depending on the angle
			double y = 0;
			if (vd.getSpeed() > 0 || !vd.isSimulation()) {
				e = degree; //degree/10.0;
				esum_str *= 0.8;
				esum_str = esum_str + e;
				Kp = 0.8; // + force/2.0;
				Ki = 0.2; //4.0 + Kp * 4.0;
				double p = Kp * e;
				//			if (degree < 0) {
				//				p = -p;
				//			}
				double i = Ki * timeStep_str * esum_str;

				y = p + i;

			}
			// max and min values for the steering force
			if (y > 20) {
				y = 38;

			}
			if (y < -38) {
				y = -38;
			}

			//Storing the previous data in case of fall-back
			old_y = y;
			old_degree = degree;

			//Averaging steering force based on previous values
			if (index > 2) {
				index = 0;
			}
			aver[index] = y;
			index++;
			double sumForce = 0;
			for (int i = 0; i < 3; i++) {
				sumForce = sumForce + aver[i];
			}

			//Send forces
			fc.setSteeringForce(sumForce / 3);
			fc.setAccelerationForce(0);

			TimeStamp currentTime_brk;
			double timeStep_brk = (currentTime_brk.toMicroseconds()
					- previousTime_brk.toMicroseconds()) / (1000.0 * 1000.0);

			previousTime_brk = currentTime_brk;

			// PI control algorithm for speed.
			if (vd.getSpeed() > 0 || !vd.isSimulation()) {
				w = MAXIMUM_SPEED;
				x = vd.getSpeed();
				e = w - x;
				esum *= 0.8; // aging factor
				esum = esum + e;
				Kp = 1.0 + abs(force) / 15.0; //2.0 + abs(y)/2.0;
				Ki = 1.0; //4.0 + Kp * 4.0;
				double p = Kp * e;
				double i = Ki * timeStep_brk * esum;
				y = p + i;
				if (y > 1.7) {
					y = 1.7;
				}
				fc.setBrakeForce(abs(y));
			}
			break;
		}
		case INTERSECTION_STATE:

			counter++;
			//Braking phase
			fc.setBrakeForce(8);
			cerr << " Counter : " << counter << endl;

			//Waiting Phase
			if (counter > 40 ){
				fc.setBrakeForce(50);
			}

			//Sensing Phase
			if (counter > 60) {
				fc.setBrakeForce(50);
				cerr << " Sensor value " << sensor << endl;
				//Case where no object is detected
				if (sensor >= 54 || sensor <= 52) {
					fc.setAccelerationForce(20);
					fc.setBrakeForce(0);
					// Change the state after accelerating for a fixed time to pass the intersection line.
					if (temp >30) {
						state = RUNNING_STATE;
						counter = 0;
						temp = 0;
					}
					temp++;
				}
			}
			break;
		}

		//Print the angle the forces
		cerr << "Angle: '" << angle << "' Steering: '" << fc.getSteeringForce()
				<< "' Acceleration: '" << fc.getAccelerationForce()
				<< "' Brake: '" << fc.getBrakeForce() << "'" << endl;
        // Set userData to 3000 => Proxy will filter ForceControl data based on this.
        fc.setUserData(3000);
		// Create container.
		Container c(Container::FORCECONTROL, fc);

		// Send container.
		getConference().send(c);

	}

	return ModuleState::OKAY;
}

} // carolocup

