#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.h"

using namespace std;

Process Scheduler::SJF_NonPreemptive(){
    
    // Latching onto an already running process, not letting it go until it's done running
    for(int i = 0; i < (int)this->processes.size(); i++){
        if(this->processes[i].getRunningState() && this->processes[i].getRemainingTime() != 0){
            this->processes[i].setRemainingTime(this->processes[i].getRemainingTime() - 1);

        if(this->processes[i].getRemainingTime() == 0){
        this->processes[i].setCompletionTime(currentTime + 1);
        this->processes[i].setRunningState(false);
        int turn_around = (currentTime + 1) - processes[i].getArrivalTime();
        processes[i].setTurnAroundTime(turn_around);
        int waiting_time = turn_around - processes[i].getBurstTime();
        processes[i].setWaitingTime(waiting_time);
    }

            return this->processes[i];
        }
    }
    
    // Picking the next process to run
    int idx = -1;
    for(int i = 0; i < (int)this->processes.size(); i++){
        if(this->processes[i].getArrivalTime() > currentTime || this->processes[i].getRemainingTime() == 0){
            continue;
        }
        else if(idx == -1){
            idx = i;
        }
        else if(this->processes[idx].getBurstTime() > this->processes[i].getBurstTime()) {
            idx = i;
        }
        
    }
    
    // Edge case, if all processes are done (won't be needed most likley bs sebo leh)
    if(idx == -1) {
        return Process("IDLE", this->currentTime, 0);
    }
    
    this->processes[idx].setRunningState(true);
    this->processes[idx].setRemainingTime(this->processes[idx].getRemainingTime() - 1);
    return this->processes[idx];
}