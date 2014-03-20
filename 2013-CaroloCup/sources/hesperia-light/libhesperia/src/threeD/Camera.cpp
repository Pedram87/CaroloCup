#include <GL/glut.h>
#include <cmath>

#include "hesperia/data/Constants.h"
#include "hesperia/threeD/Camera.h"

namespace hesperia {
    namespace threeD {

    using namespace hesperia::data;
    using namespace hesperia::data::environment;

    Camera::Camera() :
        m_position(0,0,10),
        m_orientation(0,0,0),
        m_up(0,0,0),
        m_head(static_cast<float>(-hesperia::data::Constants::PI)+0.1),
        m_pitch(static_cast<float>(-hesperia::data::Constants::PI / 4)),
        m_roll(0)
    {
    }

    Camera::~Camera() {}

    void Camera::apply()
    {
        glLoadIdentity();

        m_pitch = (m_pitch < -hesperia::data::Constants::PI) ? -hesperia::data::Constants::PI : m_pitch;
        m_pitch = (m_pitch > -0.01f) ? -0.01f : m_pitch;

        m_orientation = Point3( sin(-m_pitch) * cos(-m_head),
                                sin(-m_pitch) * sin(-m_head),
                                cos(-m_pitch) );

        m_orientation.normalize();

        m_up = Point3( sin(-m_roll) * cos(m_pitch),
                       sin(-m_roll) * sin(m_pitch),
                       cos(-m_roll) );
        m_up.normalize();

        Point3 lookAt = m_position - m_orientation;

        gluLookAt(m_position.getX(), m_position.getY(), m_position.getZ(),
                  lookAt.getX(), lookAt.getY(), lookAt.getZ(),
                  m_up.getX(), m_up.getY(), m_up.getZ());
    }

    void Camera::strafeRight(const float& length) {
        Point3 normal(m_orientation.getY(),
                      -m_orientation.getX(),
                      0);

        m_position -= normal * length;
    }

    void Camera::strafeLeft(const float& length) {
        Point3 normal(m_orientation.getY(),
                     -m_orientation.getX(),
                     0);

        m_position += normal * length;
    }

    void Camera::moveBackward(const float& length)
    {
        m_position += m_orientation * length;
    }

    void Camera::moveFoward(const float& length)
    {
        m_position -= m_orientation * length;
    }

    void Camera::moveUp(const float& length)
    {
        m_position.setZ(m_position.getZ() - (length*m_orientation.getZ()));
        //m_translationZ -= length * m_rotationZ;
    }

    void Camera::moveDown(const float& length)
    {
        m_position.setZ(m_position.getZ() + (length*m_orientation.getZ()));
        //m_translationZ += length * m_rotationZ;
    }

    void Camera::changeHead(const float& delta)
    {
        m_head -= delta;
    }

    void Camera::changePitch(const float& delta)
    {
        m_pitch -= delta;
    }

    void Camera::changeRoll(const float& delta)
    {
        m_roll -= delta;
    }

    void Camera::setPosition(const Point3& position) {
        m_position = position;
    }

    void Camera::setHead(const float& head) {
        m_head = head;

//        m_orientation = Point3( sin(-m_theta) * cos(-m_phi),
//                                sin(-m_theta) * sin(-m_phi),
//                                cos(-m_theta) );
    }

    void Camera::setPitch(const float& pitch) {
        m_pitch = pitch;

//        m_up = Point3(sin(-m_epsilon) * cos(-m_theta),
//                      sin(-m_epsilon) * sin(-m_theta),
//                      cos(-m_epsilon) );
    }

    void Camera::setRoll(const float& roll) {
        m_roll = roll;

//        m_up = Point3(sin(-m_epsilon) * cos(-m_theta),
//                      sin(-m_epsilon) * sin(-m_theta),
//                      cos(-m_epsilon) );
    }
}
};


