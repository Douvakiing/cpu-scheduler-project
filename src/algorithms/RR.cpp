#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.h"

using namespace std;

Process Scheduler::RR(int timeQuantum) {
    return Process("IDLE", currentTime, 0);
}