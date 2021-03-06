/*
 * OpenDaVINCI.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#ifndef MICEODOMETER_H_
#define MICEODOMETER_H_

#include "core/base/ConferenceClientModule.h"

#include <opencv2/video/tracking.hpp>

namespace miceodometer {

    using namespace std;

    /**
     * This class encapsulates the a mice odometer simulation module.
     */
    class MiceOdometer : public core::base::ConferenceClientModule {
        private:
            /**
             * "Forbidden" copy constructor. Goal: The compiler should warn
             * already at compile time for unwanted bugs caused by any misuse
             * of the copy constructor.
             *
             * @param obj Reference to an object of this class.
             */
            MiceOdometer(const MiceOdometer &/*obj*/);

            /**
             * "Forbidden" assignment operator. Goal: The compiler should warn
             * already at compile time for unwanted bugs caused by any misuse
             * of the assignment operator.
             *
             * @param obj Reference to an object of this class.
             * @return Reference to this instance.
             */
            MiceOdometer& operator=(const MiceOdometer &/*obj*/);

        public:
            /**
             * Constructor.
             *
             * @param argc Number of command line arguments.
             * @param argv Command line arguments.
             */
            MiceOdometer(const int32_t &argc, char **argv);

            virtual ~MiceOdometer();

            core::base::ModuleState::MODULE_EXITCODE body();

        private:
            virtual void setUp();

            virtual void tearDown();

        private:
            // Distance between the two mice.
            const double D;

            double m_phi;
            double m_d;
            double m_x;
            double m_y;
            double m_dx;
            double m_dy;

            cv::KalmanFilter m_KF;
            cv::Mat m_transitionMatrix;
            float m_processNoise;
            float m_measurementNoise;
            float m_errorCovariance;

            double m_estimatedX;
            double m_estimatedY;

            // Algorithm for calculating the vehicle's position based on readings from two mice.
            void calculatePosition(const double &deltaXLeft, const double &deltaYLeft, const double &deltaXRight, const double &deltaYRight, const double &timeStep, const double &direction);

            // Algorithm for estimating the vehicle's position using a Kalman filter.
            void initializeKalmanFilter();
            void estimatePosition(const double &timeStep);
    };

} // miceodometer

#endif /*MICEODOMETER_H_*/
