/*
* Mini-Smart-Vehicles.
*
* This software is open source. Please see COPYING and AUTHORS for further information.
*/

#ifndef LaneDetectionData_H_
#define LaneDetectionData_H_

// core/platform.h must be included to setup platform-dependent header files and configurations.
#include "core/platform.h"

#include "core/data/SerializableData.h"

#include <opencv/cv.h>

namespace msv {

	using namespace std;
	using namespace cv;

/**
* This is an example how you can send data from one component to another.
*/
	class LaneDetectionData : public core::data::SerializableData {
	public:
		LaneDetectionData();

		virtual ~LaneDetectionData();

		/**
		 * Copy constructor.
		 *
		 * @param obj Reference to an object of this class.
		 */
		LaneDetectionData(const LaneDetectionData &obj);

		/**
		 * Assignment operator.
		 *
		 * @param obj Reference to an object of this class.
		 * @return Reference to this instance.
		 */
		LaneDetectionData& operator=(const LaneDetectionData &obj);

		/**
		 * This method returns the example data.
		 *
		 * @return example data.
		 */
		double getLaneDetectionData() const;

		/**
		 * This method sets the example data.
		 *
		 * @param e Example data.
		 */
		struct Lines {
			Vec4i leftLine = Vec4i(0,0,0,0);
			Vec4i rightLine = Vec4i(0,0,0,0);
			Vec4i dashedLine = Vec4i(0,0,0,0);
		};
		void setLaneDetectionData(const Lines &e);
		virtual ostream& operator<<(ostream &out) const;
		virtual istream& operator>>(istream &in);
		virtual const string toString() const;
		private:
		Lines m_lines;
	};

} // msv

#endif /*LaneDetectionData_H_*/