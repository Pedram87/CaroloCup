/*
 * CaroloCup.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#ifndef PARKINGDRIVER_H_
#define PARKINGDRIVER_H_

#include "core/base/ConferenceClientModule.h"

namespace carolocup {

    using namespace std;

    /**
     * This class is a skeleton to send driving commands to Hesperia-light's vehicle driving dynamics simulation.
     */
    class parkingDriver : public core::base::ConferenceClientModule {
        private:
            /**
             * "Forbidden" copy constructor. Goal: The compiler should warn
             * already at compile time for unwanted bugs caused by any misuse
             * of the copy constructor.
             *
             * @param obj Reference to an object of this class.
             */
            parkingDriver(const parkingDriver &/*obj*/);

            /**
             * "Forbidden" assignment operator. Goal: The compiler should warn
             * already at compile time for unwanted bugs caused by any misuse
             * of the assignment operator.
             *
             * @param obj Reference to an object of this class.
             * @return Reference to this instance.
             */
            parkingDriver& operator=(const parkingDriver &/*obj*/);

        public:
            /**
             * Constructor.
             *
             * @param argc Number of command line arguments.
             * @param argv Command line arguments.
             */
            parkingDriver(const int32_t &argc, char **argv);

            virtual ~parkingDriver();

            core::base::ModuleState::MODULE_EXITCODE body();

        private:
            virtual void setUp();

            virtual void tearDown();
    };

} // carolocup

#endif /*PARKINGDRIVER_H_*/