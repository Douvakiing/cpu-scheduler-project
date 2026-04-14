#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.h"

using namespace std;
Process Scheduler::Priority_Preemptive() {
    int idx=-1;
    
    for(int i =0 ; i < this->processes.size(); i++)
    {
        // Skip not-yet-arrived or already finished processes.
       if(this->processes[i].getArrivalTime() > currentTime || this->processes[i].getRemainingTime() < 1)
       {
        continue;
       } 
       if(idx == -1)
       {
        idx = i;
       }
       else if(this->processes[i].getPriority() < this->processes[idx].getPriority() )
       {
        idx = i;
       }
       else if(this->processes[i].getPriority() == this->processes[idx].getPriority() &&
        this->processes[i].getArrivalTime() < this->processes[idx].getArrivalTime())
       {
        idx = i;
       }
    }
    if(idx == -1)
    {
    return Process("IDLE", currentTime, 0);
    }
    // Mark non-selected processes as not running (preempted or waiting).
    for(int i = 0 ; i < this->processes.size() ; i++)
    {
        if(i == idx)
        {
            continue;
        }
        this->processes[i].setRunningState(false); // For interrupted processes
    }

    this->processes[idx].setRunningState(true);
    this->processes[idx].setRemainingTime(this->processes[idx].getRemainingTime() - 1);
    

    // Record completion on the exact finishing tick.
    if(this->processes[idx].getRemainingTime() == 0)
    {           
        this->processes[idx].setCompletionTime(currentTime + 1);
        this->processes[idx].setRunningState(false);
        //time calculations
        int turn_around = (currentTime + 1) - this->processes[idx].getArrivalTime();
        this->processes[idx].setTurnAroundTime(turn_around);
        int waiting_time = turn_around - this->processes[idx].getBurstTime();
        this->processes[idx].setWaitingTime(waiting_time);
    }
    return this->processes[idx];
    
}
