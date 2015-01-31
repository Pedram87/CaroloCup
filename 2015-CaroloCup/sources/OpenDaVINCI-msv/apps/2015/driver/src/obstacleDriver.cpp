/*
 * Mini-Smart-Vehicles.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#include <stdio.h>
#include <math.h>
#include <sstream>
#include <fstream>

#include <pthread.h>

#include "core/io/ContainerConference.h"
#include "core/data/Container.h"
#include "core/data/Constants.h"
#include "core/data/control/VehicleControl.h"
#include "core/data/environment/VehicleData.h"
#include "core/base/LIFOQueue.h"

// Data structures from msv-data library:

#include <pthread.h>
#include "obstacleDriver.h"

int indicators = -1;
bool indicatorsOn = false;
bool overtakingNow = false;
bool overtakingDone = false;
bool obstacleDetected = false;

int arr[5][5], cirK = 0;

namespace msv
{

  using namespace std;
  using namespace core::base;
  using namespace core::data;
  using namespace core::data::control;
  using namespace core::data::environment;

// Constructor
  obstacleDriver::obstacleDriver (const int32_t &argc, char **argv) :
      ConferenceClientModule (argc, argv, "Driver"), m_hasReceivedLaneDetectionData (
	  false), after_intersection (false), m_angularError (0), m_speed (0), m_lateralError (
	  0), m_intLateralError (0), m_derLateralError (0), m_desiredSteeringWheelAngle (
	  0), m_propGain (2.05), m_intGain (8.38), m_derGain (0.23), SCALE_FACTOR (
	  752 / 0.41), m_timestamp (0), m_leftLine (Vec4i (0, 0, 0, 0)), m_rightLine (
	  Vec4i (0, 0, 0, 0)), m_dashedLine (Vec4i (0, 0, 0, 0))
  {

    for (int i = 0; i < 5; i++)
      {
	for (int j = 0; j < 5; j++)
	  {
	    arr[i][j] = 0;
	  }
      }
  }

// Destructor
  obstacleDriver::~obstacleDriver ()
  {
  }

  void
  obstacleDriver::setUp ()
  {
    // This method will be call automatically _before_ running body().
    m_speed = (0.6 * 10); //leave like this for test purpose
  }

  void
  obstacleDriver::tearDown ()
  {
    // This method will be call automatically _after_ return from body().
  }

  bool debug = true;
  int increaseSpeed = 0;

//TODO: Set indicator logic

// This method will do the main data processing job.
  ModuleState::MODULE_EXITCODE
  obstacleDriver::body ()
  {
    // Get configuration data.
    KeyValueConfiguration kv = getKeyValueConfiguration ();

    VehicleControl vc;

    bool first = true;
    while (getModuleState () == ModuleState::RUNNING)
      {

	LaneDetectionData ldd;
	SensorBoardData sdb;
	Container conUserData1 = getKeyValueDataStore ().get (
	    Container::USER_DATA_1);
	Container conUserData0 = getKeyValueDataStore ().get (
	    Container::USER_DATA_0);

	if ((conUserData1.getReceivedTimeStamp ().getSeconds ()
	    + conUserData1.getReceivedTimeStamp ().getFractionalMicroseconds ())
	    < 1)
	  {
	    cout << "New lap. Waiting..." << endl;
	    continue;
	  }
	if ((conUserData0.getReceivedTimeStamp ().getSeconds ()
	    + conUserData0.getReceivedTimeStamp ().getFractionalMicroseconds ())
	    < 1)
	  {
	    cout << "No Sensor board data" << endl;
	    continue;
	  }

	sdb = conUserData0.getData<SensorBoardData> ();
	ldd = conUserData1.getData<LaneDetectionData> ();
	if (first)
	  {
	    for (int i = 0; i < 6; i++)
	      {
		int urF = sdb.getDistance (4);
		int irSL = sdb.getDistance (0);
		int irRL = sdb.getDistance (3);
		 movingAvg (urF, 4);
		movingAvg (irSL, 0);
		movingAvg (irRL, 3);
	      }
	    first = false;
	  }

	m_propGain = 4.5; //4.5;//2.05;
	m_intGain = 0.5; //1.0;//8.39; //8.39;
	m_derGain = 0.23; //0.23;

	bool overtake = overtaking (sdb);

	//Check for intersection
	// This algo needs to be changed to work according
	// to the distance travelled in the intersection,
	// instead of the time.
	bool res = false;

	if (ldd.getLaneDetectionDataDriver ().roadState == NORMAL
	    && !after_intersection)
	  {
	    cout << "NOrmal state" << endl;
	    res = laneFollowing (&ldd, overtake);
	  }
	else if (ldd.getLaneDetectionDataDriver ().roadState == INTERSECTION
	    && ldd.getLaneDetectionDataDriver ().confidenceLevel >= 1)
	  {
	    cout << "Found Intersection..." << endl;
	    after_intersection = true;
	    TimeStamp t_start;
	    m_timestamp = t_start.toMicroseconds ();

	  }
	else if (after_intersection)
	  {
	    TimeStamp t_stop;
	    double timeStep_total = (t_stop.toMicroseconds () - m_timestamp)
		/ 1000.0;
	    cout << "TIme: " << timeStep_total << endl;
	    if (timeStep_total > 3000.0) //Cross intersect for 3 seconds
	      {
		res = laneFollowing (&ldd, overtake);
		after_intersection = false;
	      }
	    else
	      {
		vc.setSteeringWheelAngle (int16_t (0));
		cout << "Crossing Intersection..." << endl;
	      }
	  }
	else
	  {
	    cout << "U not supposed to be here: "
		<< ldd.getLaneDetectionDataDriver ().roadState << "=>"
		<< after_intersection << endl;
	  }

	if (!res)
	  {
	    cout << "Waiting..." << endl;
	    continue;
	  }

	stringstream speedStream, steeringAngleStream;

	float desSteering = m_desiredSteeringWheelAngle * 180 / M_PI;
	cout << "steeringAngle" << desSteering << endl;

	if (desSteering > 41)
	  desSteering = 42;
	if (desSteering < -41)
	  desSteering = -42;

	vc.setSteeringWheelAngle (int16_t (desSteering));

	int speedVal;
	//int runSpeed = 1565;
	speedVal = m_speed;
	if (abs (desSteering) < 4)
	  {
	    increaseSpeed++;
	  }
	else
	  {
	    increaseSpeed = 0;
	  }

	if (increaseSpeed >= 3 && increaseSpeed < 6)
	  {
	    speedVal = m_speed + 1;
	  }
	else if (increaseSpeed >= 6)
	  {
	    speedVal = m_speed + 2;
	  }

	vc.setSpeed (speedVal);

	Container c (Container::VEHICLECONTROL, vc);
	getConference ().send (c);
      }

    vc.setSpeed (0);
    vc.setSteeringWheelAngle (0);
    return ModuleState::OKAY;
  }
  bool
  obstacleDriver::overtaking (SensorBoardData sensorData)
  {
    int urF = sensorData.getDistance (4);
    int irSL = sensorData.getDistance (0);
    int irRL = sensorData.getDistance (3);
    float urF_avg = movingAvg (urF, 4);
    float irSL_avg = movingAvg (irSL, 0);
    float irRL_avg = movingAvg (irRL, 3);

    if (abs(urF_avg) < 55 && abs(urF_avg) > 1)
      {
	obstacleDetected = true;
      }
    else if (obstacleDetected)
      {
	if (abs(irSL_avg) < 20)
	  {
	    cout << "overtakingStarted" << endl;
	    if (abs(irRL_avg) < 20)
	      {
		overtakingNow = true;
	      }
	  }
	if (abs(irRL_avg) > 20 && abs(irSL_avg) > 20 && overtakingNow)
	  {
	    overtakingDone = true;
	  }
	if (overtakingDone)
	  {
	    obstacleDetected = false;
	  }
      }
    return obstacleDetected;

  }
  float
  obstacleDriver::movingAvg (int sensorVal, int pos)
  {
    float out = 0;
    arr[pos][cirK] = sensorVal;
    cirK = (cirK + 1) % 5;
    for (int i = 0; i < 5; i++)
      {
	out += arr[pos][i];
      }
    return out / 5;
  }
  float
  obstacleDriver::calculateDesiredHeading (float oldLateralError)
  {
    float desiredHeading;
    float theta = m_angularError / 180 * M_PI;
    //Scale from pixels to meters
    m_lateralError = m_lateralError / SCALE_FACTOR;
    if (m_timestamp != 0)
      {
	TimeStamp now;
	int32_t currTime = now.toMicroseconds ();
	double sec = (currTime - m_timestamp) / (1000000.0);
	m_intLateralError = m_intLateralError
	    + m_speed * cos (theta) * m_lateralError * sec;
	if ((m_intLateralError > 2 * m_lateralError && m_lateralError > 0)
	    || (m_lateralError < 0 && m_intLateralError < 2 * m_lateralError))
	  {
	    m_intLateralError = 2 * m_lateralError;
	  }
	m_derLateralError = (m_lateralError - oldLateralError) / sec;
	//cout << endl;
	//cout << "  sec: " << sec;
      }
    TimeStamp now;
    m_timestamp = now.toMicroseconds ();
    //Simple proportional control law, propGain needs to be updated
    desiredHeading = m_lateralError * m_propGain;
    desiredHeading += m_intLateralError * m_intGain;
    desiredHeading += m_derLateralError * m_derGain;
    return desiredHeading;
  }

  bool
  obstacleDriver::laneFollowing (LaneDetectionData *data, bool overtake)
  {
    cout << "enteredLaneFollowing" << endl;
    //  int x1, x2, x3, x4, y1, y2, y3, y4;
    LaneDetectionData ldd = *data;
    // The two lines are delivered in a struct containing two Vec4i objects (vector of 4 integers)
    Lines lines = ldd.getLaneDetectionData ();

    LaneDetectorDataToDriver trajectoryData = ldd.getLaneDetectionDataDriver ();

    /*if (lines.dashedLine[0] == 0 && lines.dashedLine[1] == 0 && lines.dashedLine[2] == 0 && lines.dashedLine[3] == 0)
     {
     m_leftLine = lines.leftLine;
     }
     else
     {
     m_leftLine = lines.dashedLine;
     }
     m_rightLine = lines.rightLine;*/

    // Temporary solution to stop the car if a stop line is detected
    //if (lines.stopLineHeight != -1)
    //{
    //    m_speed = 0;
    //}
    // int scr_width = lines.width;
    //int scr_height = lines.height;
    /*
     x1 = m_leftLine[0];
     y1 = m_leftLine[1];
     x2 = m_leftLine[2];
     y2 = m_leftLine[3];
     x3 = m_rightLine[0];
     y3 = m_rightLine[1];
     x4 = m_rightLine[2];
     y4 = m_rightLine[3];
     */

    if (trajectoryData.noTrajectory)
      {
	cout << "No trajectory" << endl;
	return false;
      }

    if (debug)
      {
	cout << ",propGain: " << m_propGain;
	cout << ",intGain: " << m_intGain;
	cout << ",derGain: " << m_derGain;
	cout << endl;
      }

    float oldLateralError = m_lateralError;
    CustomLine goal;
    if (overtake == true)
      {
	goal = trajectoryData.leftGoalLines0;
      }
    else
      {
	goal = trajectoryData.rightGoalLines0;
      }

    calculateErr (trajectoryData.currentLine, goal, &m_angularError,
		  &m_lateralError);

    m_desiredSteeringWheelAngle = calculateDesiredHeading (oldLateralError);
    if (debug)
      {
	// cout << "  x_error: " << x_err;
	cout << "  derLateral: " << m_derLateralError;
	cout << "  intLateral: " << m_intLateralError;
	cout << "  lateral: " << m_lateralError;
	cout << "  orentation: " << m_angularError;
	// cout << "  theta: " << theta;
	cout << "  angle: " << m_desiredSteeringWheelAngle * 180 / M_PI;
	cout << "  speed: " << m_speed;

	cout << endl;
      }
    cout << "exit lane follwoing" << endl;
    return true;
  }

// float predictHeading(int time,float currSpeed, float currHeading)
// {
//     return;

// }

  void
  obstacleDriver::calculateErr (CustomLine currLine, CustomLine goalLine,
				float *angError, double *latError)
  {
    float x_goal = goalLine.p2.x;
    float x_pl = currLine.p2.x;

    float a = tan (goalLine.slope * M_PI / 180);
    float b = goalLine.p1.y - goalLine.p1.x * a;
    int x_coord = -b / a;
    x_goal = (x_coord + x_goal) / 2;
    float theta_avg = M_PI / 2;
    if (abs (x_goal - x_pl) > 0.001)
      {
	theta_avg = (0 - currLine.p2.y) / ((float) (x_goal - x_pl));
	theta_avg = atan (theta_avg);
      }
    if (theta_avg < 0)
      {
	theta_avg = 180 + (theta_avg * 180 / M_PI);
      }
    else
      {
	theta_avg = theta_avg * 180 / M_PI;
      }

    float theta_curr = currLine.slope;
    if (debug)
      {
	cout << "Position: " << x_pl << endl;
	cout << "Goal: " << x_goal << endl;
	cout << "Curr Orientation: " << theta_curr << endl;
	cout << "Goal Orientation: " << theta_avg << endl;
      }
    *angError = theta_avg - theta_curr;
    *latError = x_goal - x_pl;

    return;
  }

} // msv

