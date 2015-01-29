/*
 * Mini-Smart-Vehicles.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#ifndef DRIVER_H_
#define DRIVER_H_

#include "core/base/ConferenceClientModule.h"
#include "core/data/control/VehicleControl.h"
#include "core/data/environment/VehicleData.h"

namespace msv {

using namespace std;
using namespace core::data::control;
using namespace core::data::environment;
enum DRIVING_STATE {
	DRIVE = 0,
	START_OBST = 1,
	POSSIBLE_SPOT = 2,
	INITIALIZE_POS_FOR_PARKING = 3,
	STOP_FOR_PARKING = 4,
	PARKING = 5,
	NO_POSSIBLE_PARKING_PLACE = 6

};

enum PARKINGSTATE {
	BACKWARDS_RIGHT = 0,
	BACKWARDS_LEFT = 1,
	WAIT_2 = 2,
	FORWARD_RIGHT = 3,
	WAIT_3 = 4,
	BACK_AGAIN = 5,
	STOP = 6,
	DONE = 7
};

/**
 * This class is a skeleton to send driving commands to Hesperia-light's vehicle driving dynamics simulation.
 */
class Driver: public core::base::ConferenceClientModule {
private:
	/**
	 * "Forbidden" copy constructor. Goal: The compiler should warn
	 * already at compile time for unwanted bugs caused by any misuse
	 * of the copy constructor.
	 *
	 * @param obj Reference to an object of this class.
	 */
	Driver(const Driver &/*obj*/);

	/**
	 * "Forbidden" assignment operator. Goal: The compiler should warn
	 * already at compile time for unwanted bugs caused by any misuse
	 * of the assignment operator.
	 *
	 * @param obj Reference to an object of this class.
	 * @return Reference to this instance.
	 */
	Driver& operator=(const Driver &/*obj*/);

public:
	/**
	 * Constructor.
	 *
	 * @param argc Number of command line arguments.
	 * @param argv Command line arguments.
	 */
	Driver(const int32_t &argc, char **argv);

	virtual ~Driver();

	core::base::ModuleState::MODULE_EXITCODE body();

private:

	DRIVING_STATE driving_state;
	PARKINGSTATE parking_state;


	virtual void setUp();

	virtual void tearDown();
	void parking();
};

} // msv

#endif /*DRIVER_H_*/