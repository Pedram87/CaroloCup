/*
 * OpenDaVINCI.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#ifndef OPENDAVINCI_CORE_WRAPPER_POSIX_POSIXCONCURRENCYFACTORYWORKER_H_
#define OPENDAVINCI_CORE_WRAPPER_POSIX_POSIXCONCURRENCYFACTORYWORKER_H_

// core/platform.h must be included to setup platform-dependent header files and configurations.
#include "core/platform.h"

#include "core/wrapper/SystemLibraryProducts.h"
#include "core/wrapper/ConcurrencyFactoryWorker.h"

#include "core/wrapper/POSIX/POSIXThread.h"

namespace core {
    namespace wrapper {

        template <> class OPENDAVINCI_API ConcurrencyFactoryWorker<SystemLibraryPosix>
        {
            public:
                static Thread* createThread(Runnable &runnable)
                {
                    return new core::wrapper::POSIX::POSIXThread(runnable);
                };

                static void usleep(const long &microseconds)
                {
                    struct timespec delay;

                    delay.tv_sec = 0;
                    delay.tv_nsec = 0;

                    const long NANOSECONDS_PER_SECOND = 1000 * 1000 * 1000;
                    long nanoseconds = microseconds * 1000;
                    while (nanoseconds >= NANOSECONDS_PER_SECOND) {
                        nanoseconds -= NANOSECONDS_PER_SECOND;
                        delay.tv_sec++;
                    }
                    // Add remaining nanoseconds.
                    delay.tv_nsec += nanoseconds;

                    nanosleep(&delay, NULL);
                };
        };
    }
} // core::wrapper::POSIX

#endif /*OPENDAVINCI_CORE_WRAPPER_POSIX_POSIXCONCURRENCYFACTORY_H_*/
