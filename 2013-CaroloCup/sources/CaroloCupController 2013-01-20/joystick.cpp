#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <iostream>
#include "joystick.h"
#include <QMetaType>

Joystick::Joystick(QObject *parent) :
    QThread(parent)
{

    mConnected = false;
    mAxis_count = 0;
    mButton_count = 0;
    mName[0] = '\0';

    qRegisterMetaType<JoystickErrorType>("JoystickErrorType");

    connect(this, SIGNAL(joystickError(int,JoystickErrorType)),
	    this, SLOT(errorSlot(int,JoystickErrorType)));
}

Joystick::~Joystick()
{
    stop();
}

Joystick::Joystick( QString& joydev, QObject *parent ) :
    QThread(parent)
{
    init( joydev );
}

void Joystick::errorSlot(int error, JoystickErrorType errorType)
{
    Q_UNUSED(error);
    Q_UNUSED(errorType);
    stop();
}

int Joystick::init( QString& joydev )
{
    stop();
    mConnected = false;
    mAxis_count = 0;
    mButton_count = 0;
    mJs_fd = open( joydev.toLocal8Bit().data(), O_RDONLY );
    if( mJs_fd < 0 )
    {
	return -1;
    }
    mDevice = joydev;
    ioctl(mJs_fd, JSIOCGAXES, &mAxis_count);
    ioctl(mJs_fd, JSIOCGBUTTONS, &mButton_count);
    ioctl(mJs_fd, JSIOCGNAME(80), &mName);
    mAxes = new int[mAxis_count];
    mButtons = new char(mButton_count);
    fcntl(mJs_fd, F_SETFL, O_NONBLOCK);
    mConnected = true;

    mAbort = false;
    start(LowPriority);
    return 0;
}

void Joystick::stop()
{
    mMutex.lock();
    mAbort = true;
    mMutex.unlock();

    wait();

    mMutex.lock();
    if(mConnected)
    {
        close(mJs_fd);
        mConnected = false;
        mAxis_count = 0;
        mButton_count = 0;
        delete mAxes;
        delete mButtons;
    }
    mMutex.unlock();
}

char Joystick::getButton( int button )
{
    if(button < (mButton_count))
    {
	QMutexLocker locker(&mMutex);
        return (mButtons)[button];
    }
    return -1;
}

int Joystick::getAxis( int axis )
{
    if(axis < mAxis_count)
    {
	QMutexLocker locker(&mMutex);
        return mAxes[axis];
    }
    return -65535;
}

QString Joystick::getName()
{
    return QString(mName);
}

QString Joystick::getDevice()
{
    return mDevice;
}

int Joystick::numButtons()
{
    return mButton_count;
}

int Joystick::numAxes()
{
    return mAxis_count;
}

bool Joystick::isConnected()
{
    return mConnected;
}

void Joystick::run()
{
    struct js_event event;
    fd_set set;
    timeval timeout;
    int rv;

    while (false == mAbort)
    {
        FD_ZERO(&set); /* clear the set */
        FD_SET(mJs_fd, &set); /* add our file descriptor to the set */
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;
        rv = select(mJs_fd + 1, &set, NULL, NULL, &timeout);

	if(rv < 0)
        {
            // Some error occured
	    qCritical().nospace() << "Joystick select read error: " << rv;
	    Q_EMIT joystickError(rv, JOYSTICK_ERROR_SELECT_READ);
        }

        else if(rv == 0)
        {
            // Timeout
        }

        else
        {
	   rv = read(mJs_fd, &event, sizeof(struct js_event));
           switch ((event.type) & ~JS_EVENT_INIT)
           {
           case JS_EVENT_AXIS:
               mMutex.lock();
               mAxes[event.number] = event.value;
               mMutex.unlock();
               break;
           case JS_EVENT_BUTTON:
               mMutex.lock();
               mButtons[event.number] = event.value;
               mMutex.unlock();
	       if (event.value == 0 && !(event.type & JS_EVENT_INIT))
               {
		   Q_EMIT buttonPressed(event.number);
               }
               break;
           }

	   if(rv < 0)
	   {
	       // Some error occured
	       qCritical().nospace() << "Joystick read error: " << rv;
	       Q_EMIT joystickError(rv, JOYSTICK_ERROR_READ);

	   }
        }
    }
}
