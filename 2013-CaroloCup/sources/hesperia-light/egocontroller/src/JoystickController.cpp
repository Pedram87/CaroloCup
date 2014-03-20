/*
 * Copyright (c) Christian Berger and Bernhard Rumpe, Rheinisch-Westfaelische Technische Hochschule Aachen, Germany.
 *
 * The Hesperia Framework.
 */

#include "JoystickController.h"

#ifndef WIN32
#include <cstdlib>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#endif

namespace egocontroller {

    using namespace std;
    using namespace hesperia::data::environment;

    JoystickController::JoystickController(ControlBehaviour& behaviour, const string& device) :
        m_behaviour(behaviour),
        MAX(32768.0),
        FACTOR_ACCELERATION(80),
        FACTOR_ROTATION(30),
        m_joy_fd(0),
        m_axes(NULL),
        m_lastAxis0(0.0),
        m_lastAxis1(0.0)
    {
        int num_of_axes = 0;
        int num_of_buttons = 0;

        int name_of_joystick[80];

        if ((m_joy_fd = open(device.c_str() , O_RDONLY)) == -1) {
            cerr << "Could not open joystick " << device << endl;
            return;
        }

        ioctl(m_joy_fd, JSIOCGAXES, &num_of_axes);
        ioctl(m_joy_fd, JSIOCGBUTTONS, &num_of_buttons);
        ioctl(m_joy_fd, JSIOCGNAME(80), &name_of_joystick);

        m_axes = (int *)calloc(num_of_axes, sizeof(int));
        cerr << "Joystick found " << name_of_joystick << ", number of axes: " << num_of_axes << ", number of buttons: " << num_of_buttons <<endl;

        // Use non blocking reading.
        fcntl(m_joy_fd, F_SETFL, O_NONBLOCK);
    }

    JoystickController::~JoystickController()
    {
        close(m_joy_fd);
    }

    void JoystickController::doWork()
    {
        struct js_event js;
        read(m_joy_fd, &js, sizeof(struct js_event));

        // Check event.
        switch (js.type & ~JS_EVENT_INIT) {
            case JS_EVENT_AXIS:
                m_axes[js.number] = js.value;
            break;
        }

        double acceleration = 0;
        double deceleration = 0;
        double rotation = 0;
        if (m_axes[1] > 0) {
            // Braking.
            deceleration = (m_axes[1]/MAX)*FACTOR_ACCELERATION;
            m_behaviour.brake(deceleration);
        }
        else {
            // Accelerating.
            acceleration = ((-m_axes[1])/MAX)*FACTOR_ACCELERATION;
            m_behaviour.accelerate(acceleration);
        }

        // Steering.
        rotation = (m_axes[0]/MAX)*FACTOR_ROTATION;
        m_behaviour.turnRight(rotation);

        cerr << "Values: A: " << acceleration << ", B: " << deceleration << ", R: " << rotation << endl;
    }

    EgoState JoystickController::getEgoState()
    {
        return m_behaviour.computeEgoState();
    }
}
