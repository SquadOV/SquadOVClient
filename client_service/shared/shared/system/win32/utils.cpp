#ifdef _WIN32

#include <Windows.h>
#include <processthreadsapi.h>
#include "shared/system/utils.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include "shared/math.h"

namespace shared::system::utils {

TimePrecisionInitializer::TimePrecisionInitializer() {
    // Starting with Windows 11, if a window-owning process becomes minimized Windows
    // will ignore our requested timer resolution and go back to the system default. SquadOV
    // is a window-owning process unfortunately. We need to make sure that the flag
    // PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION is explictly turned OFF to prevent this behavior.
    PROCESS_POWER_THROTTLING_STATE throttle = { 0 };
    throttle.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    throttle.ControlMask = PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION;
    throttle.StateMask = 0;
    SetProcessInformation(
        GetCurrentProcess(), 
        ProcessPowerThrottling, 
        &throttle, 
        sizeof(throttle)
    );

    timeBeginPeriod(1);
}

TimePrecisionInitializer::~TimePrecisionInitializer() {
    timeEndPeriod(1);
}

void preciseSleep(int64_t nanoseconds) {
    // This function relies on the TimePrecisionInitializer object being created on program startup.
    // The code here is generally based off the "Accurate FPS Limiting / High-precision 'Sleeps'" code found here:
    // http://www.geisswerks.com/ryan/FAQS/timing.html

    // pcFreq is in counts per second.
    static LARGE_INTEGER pcFreq = [](){
        LARGE_INTEGER p;
        QueryPerformanceFrequency(&p);
        return p;
    }();

    // We need to convert the nanoseconds that came in to a number of ticks we want to wait as determined by the
    // performance counter from Windows. So what we need to do is:
    //  ticks = nanoseconds / 1e+9 * pcFreq .
    const auto ticksToWait = shared::math::i64MulDiv(nanoseconds, pcFreq.QuadPart, 1000000000);

    LARGE_INTEGER targetTick;
    QueryPerformanceCounter(&targetTick);
    targetTick.QuadPart += ticksToWait;

    // The code by geisswerks does a loop of Sleep's - though I can't see an obvious reason to sleep in increments of 1 rather than
    // doing the Sleep in one go - given that we set timeBeginPeriod I'm assuming we'll get something similar to the amount of time we want to sleep.
    const auto milliseconds = nanoseconds / 1000000;

    // According to Geisswerks, Sleep(1) will sleep 1-2ms, Sleep(2) will sleep 2-3ms, etc. So by that logic, we need to account for
    // a scenario where the Sleep will sleep an extra millisecond (which we don't want to happen) so we only use the sleep if we want to sleep
    // for at least 2 milliseconds.
    if (milliseconds > 2) {
        Sleep(milliseconds - 1);
    }

    LARGE_INTEGER t;
    while (true) {
        QueryPerformanceCounter(&t);

        if (t.QuadPart >= targetTick.QuadPart) {
            break;
        }

        // We just want to give up the timeslice here. Geisswerks used multiple sleeps to accomplish
        // moving forward some small increment in time. To be fully accurate, let's only do a single sleep.
        Sleep(0);
    }
}

}


#endif