#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.h"

using namespace std;

Process Scheduler::RR(int timeQuantum) {
    
    // 1. Add newly arrived processes to the ready queue
    // We check if it arrived exactly at the current tick to add it.
    for (int i = 0; i < (int)processes.size(); i++) {
        if (processes[i].getArrivalTime() == currentTime && processes[i].getRemainingTime() > 0) {
            rr_ready.push_back(i);
        }
    }

    // 2. Manage the currently running process from the previous tick
    if (rr_running != -1) {
        if (processes[rr_running].getRemainingTime() == 0) {
            // Process finished on the last tick. Free the CPU.
            rr_running = -1;
        } else if (rr_time_left_in_quantum == 0) {
            // Quantum expired on the last tick but process isn't finished.
            // Preempt it and push it to the BACK of the ready queue.
            processes[rr_running].setRunningState(false);
            rr_ready.push_back(rr_running);
            rr_running = -1;
        }
    }

    // 3. If CPU is idle, pick the next process from the front of the ready queue
    if (rr_running == -1) {
        if (!rr_ready.empty()) {
            rr_running = rr_ready.front();
            rr_ready.pop_front();
            
            processes[rr_running].setRunningState(true);
            rr_time_left_in_quantum = timeQuantum;
        }
    }

    // 4. Execute the currently running process (if any)
    if (rr_running != -1) {
        processes[rr_running].setRemainingTime(processes[rr_running].getRemainingTime() - 1);
        rr_time_left_in_quantum--;

        // ---> Calculate metrics if the process finishes on THIS exact tick
        if (processes[rr_running].getRemainingTime() == 0) {
            processes[rr_running].setCompletionTime(currentTime + 1);
            processes[rr_running].setRunningState(false);
            
            int turn_around = (currentTime + 1) - processes[rr_running].getArrivalTime();
            processes[rr_running].setTurnAroundTime(turn_around);
            
            int waiting_time = turn_around - processes[rr_running].getBurstTime();
            processes[rr_running].setWaitingTime(waiting_time);
        }

        return processes[rr_running];
    }

    // 5. If no process is ready, the CPU is idle
    return Process("IDLE", currentTime, 0);
}