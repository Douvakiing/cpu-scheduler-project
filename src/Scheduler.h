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
    void removeProcess(int index);
    void displayProcesses() const;
    void clearProcesses();

    int getCurrentTime() const { return currentTime; }
    void setCurrentTime(int t) { currentTime = t; }
    void advanceTime() { currentTime++; }

    vector<Process>& getProcesses() { return processes; }
    const vector<Process>& getProcesses() const { return processes; }

    bool allProcessesFinished() const;

    // One simulation time unit each; caller should advanceTime() after each call.
    Process FCFS();
    Process SJF_Premetive();
    Process SJF_NonPremetive();
    Process RR(int timeQuantum);
    Process Priority_Premetive();
    Process Priority_NonPremetive();
};

#endif
