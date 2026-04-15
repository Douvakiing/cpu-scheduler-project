#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <deque>
#include <iostream>
#include <vector>
#include "Process.h"
using namespace std;

class Scheduler {
private:
    vector<Process> processes;
    int currentTime = 0;

    deque<int> rr_ready;
    int rr_running = -1;
    int rr_time_left_in_quantum = 0;

    void resetRoundRobinState();

public:
    Scheduler();

    void addProcess(const Process& process, int index = -1);
    void removeProcess(int index);
    void displayProcesses() const;
    void clearProcesses();

    int getCurrentTime() const { return currentTime; }
    void setCurrentTime(int t) { currentTime = t; }
    void advanceTime();

    vector<Process>& getProcesses() { return processes; }
    const vector<Process>& getProcesses() const { return processes; }

    bool allProcessesFinished() const;

    Process FCFS();
    Process SJF_Preemptive();
    Process SJF_NonPreemptive();
    Process RR(int timeQuantum);
    Process Priority_Preemptive();
    Process Priority_NonPreemptive();

    double avgwWaitTime();
    double avgTAT();
};

#endif
