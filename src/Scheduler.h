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
    void advanceTime() { currentTime++; }

    vector<Process>& getProcesses() { return processes; }
    const vector<Process>& getProcesses() const { return processes; }

    bool allProcessesFinished() const;

    Process FCFS();
    Process SJF_Premetive();
    Process SJF_NonPremetive();
    void RR(int timeQuantum);
    void Priority_Premetive();
    void Priority_NonPremetive();
};

#endif
