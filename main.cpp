#include <iostream>
using namespace std;

#include "Process.h"
#include "Scheduler.h"

int main() {
    Process p1 = Process("P1", 0, 10, 2);
    Process p2 = Process("P2", 2, 4, 1);

    Scheduler scheduler;
    scheduler.addProcess(p1);
    scheduler.addProcess(p2);
    scheduler.displayProcesses();
}