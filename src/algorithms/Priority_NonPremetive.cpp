#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.h"

using namespace std;

Process Scheduler::Priority_NonPremetive() {
    return Process("IDLE", currentTime, 0);
}