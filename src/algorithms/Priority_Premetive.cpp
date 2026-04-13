#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.h"

using namespace std;
int time_counter =0;
Process Scheduler::Priority_Premetive() {
    int idx=-1;
    time_counter++;
    
    for(int i =0 ; i < this->processes.size(); i++)
    {
        // Skips finised processes
       if( this->processes[i].getRemainingTime() < 1)
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
    //Updating the waiting time and running states(for Preemptive) of the unchoosen unfinished processes
    for(int i = 0 ; i < this->processes.size() ; i++)
    {
        if(i == idx || this->processes[i].getRemainingTime() == 0)
        {
            continue;
        }
        this->processes[i].setWaitingTime((this->processes[i].getWaitingTime()) + 1);
        this->processes[i].setRunningState(false); // For interrupted processes
    }

    this->processes[idx].setRunningState(true);
    this->processes[idx].setRemainingTime(this->processes[idx].getRemainingTime() - 1);

    //Setting the Completion and turn around times after the process finishes execution
    if(this->processes[idx].getRemainingTime() == 0)
    {
        this->processes[idx].setCompletionTime(time_counter);
        this->processes[idx].setTurnAroundTime(time_counter - this->processes[idx].getArrivalTime());
    }
    return this->processes[idx];
    
}
