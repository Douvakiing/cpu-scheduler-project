#include "Scheduler.h"

Scheduler::Scheduler() : currentTime(0) {}

void Scheduler::resetRoundRobinState() {
    rr_ready.clear();
    rr_running = -1;
    rr_time_left_in_quantum = 0;
}

bool Scheduler::allProcessesFinished() const {
    for (const Process& p : processes) {
        if (p.getRemainingTime() > 0) {
            return false;
        }
    }
    return true;
}

void Scheduler::addProcess(const Process& process, int index) {
    if (index == -1) {
        processes.push_back(process);
        return;
    }
    const int n = static_cast<int>(processes.size());
    if (index < 0 || index > n) {
        cout << "Invalid index. Process not added." << endl;
        return;
    }
    processes.insert(processes.begin() + index, process);
}

void Scheduler::removeProcess(int index) {
    if (index < 0 || index >= static_cast<int>(processes.size())) {
        cout << "Invalid index. Process not removed." << endl;
        return;
    }
    processes.erase(processes.begin() + index);
}

void Scheduler::displayProcesses() const {
    cout << "Current Processes in Scheduler:" << endl;
    for (size_t i = 0; i < processes.size(); ++i) {
        cout << "Index " << i << ": " << processes[i].getName() << endl;
    }
}


void Scheduler::updateWaitTime(){ 
    for(int i = 0; i < (int)this->processes.size(); i++){
        if(this->processes[i].getArrivalTime() > currentTime || 
        this->processes[i].getRemainingTime() == 0){
            continue;
        } 
        else{
            if(!this->processes[i].getRunningState()){
                this->processes[i].setWaitingTime(this->processes[i].getWaitingTime() + 1);
            }
        }
    }
}

void Scheduler::updateTAT(){
    for(int i = 0; i < (int)this->processes.size(); i++){
        if(this->processes[i].getCompletionTime() != -1){
            this->processes[i].setTurnAroundTime(this->processes[i].getCompletionTime() - this->processes[i].getArrivalTime());
        } 
    }
}

double Scheduler::avgwWaitTime(){
    if(processes.empty()) return 0;

    double totalWaiting = 0;
    for(int i = 0; i < this->processes.size(); i++){
        totalWaiting += this->processes[i].getWaitingTime();
    }

    return (totalWaiting / this->processes.size());
}

double Scheduler::avgTAT(){
    if(processes.empty()) return 0;

    double totalTAT = 0;
    for(int i = 0; i < this->processes.size(); i++){
        totalTAT += this->processes[i].getTurnAroundTime();
    }

    return (totalTAT / this->processes.size());
}


void Scheduler::advanceTime() { 
    this->currentTime++; 

    updateWaitTime();
    updateTAT();
    
}

void Scheduler::clearProcesses() {
    processes.clear();
    resetRoundRobinState();
}