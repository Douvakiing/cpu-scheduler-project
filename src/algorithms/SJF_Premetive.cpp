#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.h"

using namespace std;

Process Scheduler::SJF_Premetive() {
    return Process("IDLE", currentTime, 0);
}