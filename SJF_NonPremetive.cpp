#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.h"
#include "Scheduler.cpp"

using namespace std;

Process Scheduler::SJF_NonPremetive(){
    int idx = -1;
    for(int i = 0; i < (int)this->processes.size(); i++){
        if(this->processes[i].getRunningState() && this->processes[i].getRemainingTime() != 0){
            this->processes[i].setRemainingTime(this->processes[i].getRemainingTime() - 1);
            currentTime++;
            return this->processes[i];
        }
        else if(this->processes[i].getRunningState() && this->processes[i].getRemainingTime() == 0){
            this->processes[i].setRunningState(false);
        }
    }

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

    if(idx == -1) {
        currentTime++;
        return Process("IDLE", currentTime, 0);
    }
    
    this->processes[idx].setRunningState(true);
    this->processes[idx].setRemainingTime(this->processes[idx].getRemainingTime() - 1);
    currentTime++;
    return this->processes[idx];
}