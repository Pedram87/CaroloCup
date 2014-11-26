/*
 * Mini-Smart-Vehicles.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 *
 *
 */

#include <stdio.h>
#include <math.h>

#include "core/io/ContainerConference.h"
#include "core/data/Container.h"
#include "core/data/Constants.h"
#include "core/data/control/VehicleControl.h"
#include "core/data/environment/VehicleData.h"

// Data structures from msv-data library:
#include "SteeringData.h"
#include "SensorBoardData.h"
#include "UserButtonData.h"

#include "Driver.h"

namespace msv {

using namespace std;
using namespace core::base;
using namespace core::data;
using namespace core::data::control;
using namespace core::data::environment;

double MinParkingDist = 7;

Driver::Driver(const int32_t &argc, char **argv) :
		ConferenceClientModule(argc, argv, "Driver"), driving_state(DRIVE), parking_state(
				INIT_PARKING) {
}

Driver::~Driver() {
}

void Driver::setUp() {
	// This method will be call automatically _before_ running body().
}

void Driver::tearDown() {
	// This method will be call automatically _after_ return from body().
}


double start_timer;
double time_taken;

double driving_speed;			// Speed of the car
double desiredSteeringWheelAngle;// Angle of the wheels

// This method will do the main data processing job.
ModuleState::MODULE_EXITCODE Driver::body() {

	driving_state = DRIVE;
	parking_state = INIT_PARKING;
	// Get configuration data.
	KeyValueConfiguration kv = getKeyValueConfiguration();
	//const uint32_t m_sensorId = kv.getValue<int32_t> ("irus.sensor2.id");
	//cout << "***********  Sensor ID: " << m_sensorId << endl;

	VehicleControl vc;

	TimeStamp start;


	while (getModuleState() == ModuleState::RUNNING) {
		// In the following, you find example for the various data sources that are available:

		// 1. Get most recent vehicle data:
		Container containerVehicleData = getKeyValueDataStore().get(
				Container::VEHICLEDATA);
		VehicleData vd = containerVehicleData.getData<VehicleData>();
		cerr << "Most recent vehicle data: '" << vd.toString() << "'" << endl;

		// 2. Get most recent sensor board data:
		Container containerSensorBoardData = getKeyValueDataStore().get(
				Container::USER_DATA_0);
		SensorBoardData sbd =
				containerSensorBoardData.getData<SensorBoardData>();
		cerr << "Most recent sensor board data: '" << sbd.toString() << "'"
				<< endl;

		// 3. Get most recent user button data:
		Container containerUserButtonData = getKeyValueDataStore().get(
				Container::USER_BUTTON);
		UserButtonData ubd = containerUserButtonData.getData<UserButtonData>();
		cerr << "Most recent user button data: '" << ubd.toString() << "'"
				<< endl;

		// 4. Get most recent steering data as fill from lanedetector for example:
		Container containerSteeringData = getKeyValueDataStore().get(
				Container::USER_DATA_1);
		SteeringData sd = containerSteeringData.getData<SteeringData>();
		cerr << "Most recent steering data: '" << sd.toString() << "'" << endl;

		double IRdist = sbd.getDistance(2);
		cout << " ===== Infrared reading: " << IRdist << endl;

		// Design your control algorithm here depending on the input data from above.
		switch (driving_state) {
		case DRIVE: {
			cout << "In drive mode" << endl;
			driving_speed = 1.0;
			desiredSteeringWheelAngle = 0;

			if (IRdist > 0)
				driving_state = START_OBST;
		}
			break;

		case START_OBST: {
			cout << "Scanning Obstacle" << endl;
			driving_speed = 1.0;
			desiredSteeringWheelAngle = 0;
			if (IRdist < 0) {
				driving_state = POSSIBLE_SPOT;
				TimeStamp currentTime_strt1;
				start = currentTime_strt1;
			}
		}
			break;

		case POSSIBLE_SPOT: {
			TimeStamp currentTime;
			double m_time = (currentTime.toMicroseconds()
					- start.toMicroseconds()) / 1000000.0;

			double distance = vd.getSpeed() * m_time;
			cout << "---- DIstance so far: " << distance << endl;
			driving_speed = 1.0;
			desiredSteeringWheelAngle = 0;

			if (IRdist > 0) {
				driving_state = START_OBST;
			} else if (distance > MinParkingDist) {

				driving_speed = 0;
				desiredSteeringWheelAngle = 0;
				driving_state = STOP_FOR_PARKING;
				start_timer = currentTime.toMicroseconds() / 1000000.0;

				cout << "FOund a parking spot" << endl;
			}

			cout << "Found a possible spot" << endl;
		}
			break;

		case STOP_FOR_PARKING: {

			TimeStamp currentTime2;
			time_taken = (currentTime2.toMicroseconds() / 1000000.0)
					- start_timer;
			cout << "++++++++++ Stoping timer: " << time_taken << endl;

			if (time_taken > 4) {

				driving_state = PARKING;
				start_timer = currentTime2.toMicroseconds() / 1000000.0;

			}
		}
			break;

		case PARKING: {
			parking(start_timer, vc, vd);
		}
			break;
		default: {

			cout << "Non of these states" << endl;

			driving_speed = 4;
			desiredSteeringWheelAngle = 0;

		}
		}

		// Create vehicle control data.

		// With setSpeed you can set a desired speed for the vehicle in the range of -2.0 (backwards) .. 0 (stop) .. +2.0 (forwards)
		vc.setSpeed(driving_speed);

		// With setSteeringWheelAngle, you can steer in the range of -26 (left) .. 0 (straight) .. +25 (right)
		//double desiredSteeringWheelAngle = 0; // 4 degree but SteeringWheelAngle expects the angle in radians!
		vc.setSteeringWheelAngle(
				desiredSteeringWheelAngle * Constants::DEG2RAD);

		// You can also turn on or off various lights:
		vc.setBrakeLights(false);
		vc.setLeftFlashingLights(false);
		vc.setRightFlashingLights(true);

		// Create container for finally sending the data.
		Container c(Container::VEHICLECONTROL, vc);
		// Send container.
		getConference().send(c);
	}

	return ModuleState::OKAY;
}


void Driver::parking(double time, VehicleControl& vc, VehicleData& vd) {

	switch (parking_state) {
	case INIT_PARKING: {

		TimeStamp currentTime3;
		time_taken = (currentTime3.toMicroseconds() / 1000000.0)
				- time;
		double distance = -vd.getSpeed() * time_taken;

		if (distance > 7) {
			parking_state = BACK_UP_PARKING;
			driving_speed = 0;
			desiredSteeringWheelAngle = 0;
			TimeStamp currentTime4;
			start_timer = currentTime4.toMicroseconds() / 1000000.0;

		} else {
			driving_speed = -0.4;
			desiredSteeringWheelAngle = 55;
			cout << "%%%%%%  backing distance:  " << distance << endl;
		}

		cout << "##### Actuall streering wheel angle: "
				<< (double) vc.getSteeringWheelAngle() << endl;

		cout << "Initiating Parking" << endl;
	}
		break;
	case BACK_UP_PARKING: {

		TimeStamp currentTime5;
		time_taken = (currentTime5.toMicroseconds() / 1000000.0) - start_timer;
		double distance = -vd.getSpeed() * time_taken;

		if (distance < 3) {

			driving_speed = -0.4;
			desiredSteeringWheelAngle = -55;
			cout << "%%%%%%  backing distance:  " << distance << endl;

		} else {
			driving_speed = 0;
			desiredSteeringWheelAngle = 0;
			TimeStamp currentTime6;
			start_timer = currentTime6.toMicroseconds() / 1000000.0;
			parking_state = FINAL_PARKING;
			cout << "%%%%%%  stop the car:  " << endl;
		}
	}
		break;

	case FINAL_PARKING: {

		TimeStamp currentTime7;
		time_taken = (currentTime7.toMicroseconds() / 1000000.0) - start_timer;
		double distance = vd.getSpeed() * time_taken;

		if (distance < 3.5) {

			driving_speed = 0.4;
			desiredSteeringWheelAngle = 55;
			cout << "%%%%%%  forward distance:  " << distance << endl;
		} else {
			driving_speed = 0;
			desiredSteeringWheelAngle = 0;

			start_timer = currentTime7.toMicroseconds() / 1000000.0;
			parking_state = BACK_AGAIN;
			cout << "%%%%%%  forward distance:  " << distance << endl;
		}

	}

		break;

	case BACK_AGAIN: {

		TimeStamp currentTime8;
		time_taken = (currentTime8.toMicroseconds() / 1000000.0) - start_timer;
		double distance = -vd.getSpeed() * time_taken;

		if (distance < 2) {

			driving_speed = -0.4;
			desiredSteeringWheelAngle = -15;
			cout << "%%%%%%  back again distance:  " << distance << endl;

		} else {
			driving_speed = 0;
			desiredSteeringWheelAngle = 0;

			parking_state = STOP;
			cout << "%%%%%%  going back stop:  " << distance << endl;
		}

	}

		break;

	case STOP:
		driving_speed = 0;
		desiredSteeringWheelAngle = 0;

		cout << "****  stop the car  ****" << endl;

		break;
	default: {

		cout << "Non of these states" << endl;

		//driving_speed = 4;
		//desiredSteeringWheelAngle = 0;

	}
	}
}

} // msv
