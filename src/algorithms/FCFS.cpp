#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.h"

using namespace std;

Process Scheduler::FCFS(){
    
    // 1. Latch onto an already running process, not letting it go until it's done running (Non-preemptive)
    for(int i = 0; i < (int)this->processes.size(); i++){
        if(this->processes[i].getRunningState() && this->processes[i].getRemainingTime() != 0){
            this->processes[i].setRemainingTime(this->processes[i].getRemainingTime() - 1);
            if(this->processes[i].getRemainingTime() == 0){
                this->processes[i].setCompletionTime(currentTime + 1);
                this->processes[i].setRunningState(false);
                int turn_around = (currentTime + 1) - this->processes[i].getArrivalTime();
                this->processes[i].setTurnAroundTime(turn_around);
                int waiting_time = turn_around - this->processes[i].getBurstTime();
                this->processes[i].setWaitingTime(waiting_time);
            }
            return this->processes[i];
        }
        else if(this->processes[i].getRunningState() && this->processes[i].getRemainingTime() == 0){
            this->processes[i].setRunningState(false);
        }
    }
    
    // 2. Pick the next process to run based on FCFS (Earliest Arrival Time)
    int idx = -1;
    for(int i = 0; i < (int)this->processes.size(); i++){
        // Skip processes that haven't arrived yet or have already finished
        if(this->processes[i].getArrivalTime() > currentTime || this->processes[i].getRemainingTime() == 0){
            continue;
        }
        // If we haven't picked a process yet, pick the first one we find
        else if(idx == -1){
            idx = i;
        }
        // FCFS LOGIC: If we find a process that arrived EARLIER than the current best candidate, select it instead.
        else if(this->processes[i].getArrivalTime() < this->processes[idx].getArrivalTime()) {
            idx = i;
        }
    }

    // 3. Edge case: if no process is ready to run, the CPU is idle for this tick
    if(idx == -1) {
        return Process("IDLE", currentTime, 0);
    }
    
    // 4. Start the selected process and decrement its remaining time by 1 unit
    this->processes[idx].setRunningState(true);
    this->processes[idx].setRemainingTime(this->processes[idx].getRemainingTime() - 1);

    if(this->processes[idx].getRemainingTime() == 0){
        this->processes[idx].setCompletionTime(currentTime + 1);
        this->processes[idx].setRunningState(false);
        int turn_around = (currentTime + 1) - this->processes[idx].getArrivalTime();
        this->processes[idx].setTurnAroundTime(turn_around);
        int waiting_time = turn_around - this->processes[idx].getBurstTime();
        this->processes[idx].setWaitingTime(waiting_time);
    }
    
    return this->processes[idx];
}