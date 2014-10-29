/*
 * Mini-Smart-Vehicles.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#ifndef LANEDETECTOR_H_
#define LANEDETECTOR_H_

#include <opencv/cv.h>
#include "core/SharedPointer.h"
#include "core/base/ConferenceClientModule.h"
#include "core/wrapper/SharedMemory.h"

#include <LineDetector.h>

namespace msv {

    using namespace std;

    /**
     * This class is an exemplary skeleton for processing video data.
     */
    class LaneDetector_inspection: public core::base::ConferenceClientModule {
        private:
	        /**
	         * "Forbidden" copy constructor. Goal: The compiler should warn
	         * already at compile time for unwanted bugs caused by any misuse
	         * of the copy constructor.
	         *
	         * @param obj Reference to an object of this class.
	         */
	        LaneDetector_inspection(const LaneDetector_inspection &/*obj*/);

	        /**
	         * "Forbidden" assignment operator. Goal: The compiler should warn
	         * already at compile time for unwanted bugs caused by any misuse
	         * of the assignment operator.
	         *
	         * @param obj Reference to an object of this class.
	         * @return Reference to this instance.
	         */
	        LaneDetector_inspection& operator=(const LaneDetector_inspection &/*obj*/);

        public:
	        /**
	         * Constructor.
	         *
	         * @param argc Number of command line arguments.
	         * @param argv Command line arguments.
	         */
	        LaneDetector_inspection(const int32_t &argc, char **argv);

	        virtual ~LaneDetector_inspection();

	        core::base::ModuleState::MODULE_EXITCODE body();

        protected:
	        /**
	         * This method is called to process an incoming container.
	         *
	         * @param c Container to process.
	         * @return true if c was successfully processed.
	         */
	        bool readSharedImage(core::data::Container &c);

        private:
	        bool m_hasAttachedToSharedImageMemory;
	        core::SharedPointer<core::wrapper::SharedMemory> m_sharedImageMemory;
	        uint32_t m_cameraId;
            bool m_debug;
            Config m_config;
            Mat m_frame;
            uint32_t m_frame_count;
	        

	        virtual void setUp();

	        virtual void tearDown();

            void processImage();
    };

} // msv

#endif /*LANEDETECTOR_H_*/
