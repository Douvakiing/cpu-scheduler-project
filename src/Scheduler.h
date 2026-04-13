#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <vector>
#include "Process.h"
using namespace std;

class Scheduler {
private:
    vector<Process> processes;
    int currentTime = 0;
public:
    Scheduler();

    void addProcess(const Process& process, int index = -1);
    void removeProcess(int pid);
    void displayProcesses() const;
    void clearProcesses();

    void FCFS();
    void SJF_Premetive();
    Process SJF_NonPremetive();
    void RR(int timeQuantum);
    void Priority_Premetive();
    void Priority_NonPremetive();

    int calculateWaitingTime(); // Per Process
    int calculateTurnAroundTime(); // Per Process
    int calculateAvgWait(); // For all processes
    int calculateAvgTurnAround(); // For all processes
};

#endif