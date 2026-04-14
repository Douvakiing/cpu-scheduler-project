#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.h"

using namespace std;

Process Scheduler::SJF_Preemptive() {
    int next_index = -1;
    int min_rem = INT_MAX;
    //find the process with shortest rem time
    for(int i =0; i<processes.size(); i++){
        if(processes[i].getArrivalTime() <= this->getCurrentTime()
            && processes[i].getRemainingTime() < min_rem
        && processes[i].getRemainingTime() > 0 ){
                //clear running state for each process
                processes[i].setRunningState(false);
                //update min remaining time found and index
                min_rem = processes[i].getRemainingTime();
                next_index = i;
        }
    }
    //return the found process and update the remaining time and running state
    if(next_index >= 0){

        int rem_time = processes[next_index].getRemainingTime() -1;
        processes[next_index].setRemainingTime(rem_time);

        if(rem_time > 0){
            processes[next_index].setRunningState(true);
        }
        else{
            processes[next_index].setRunningState(false);
            int time = this ->currentTime;
            //set completion time current time +1
            processes[next_index].setCompletionTime(time + 1);
            //set turnaround time completion - arrival
            int turn_around = (time + 1) - processes[next_index].getArrivalTime();
            processes[next_index].setTurnAroundTime(turn_around);
            //set waiting time turn around - burst
            int waiting_time = turn_around - processes[next_index].getBurstTime();
            processes[next_index].setWaitingTime(waiting_time);

        }

        return processes[next_index];
    } 

    return Process("IDLE", currentTime, 0);
}