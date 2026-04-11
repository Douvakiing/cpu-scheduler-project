#ifndef PROCESS_H
#define PROCESS_H

#include <iostream>
#include <string>
using namespace std;

class Process{
private:
    inline static int idCounter;
    int pId;
    string name;
    int arrivalTime;
    int burstTime;
    int priority;

    int remainingTime;
    int waitingTime;
    int turnAroundTime;
    int completionTime;

    bool running;

public:
    Process(string name, int arrivalTime, int burstTime, int priority=0, bool running=false){
        this->pId = idCounter++;
        this->name = name;
        this->arrivalTime = arrivalTime;
        this->burstTime = burstTime;
        this->priority = priority;
        this->remainingTime = burstTime;
        this->running = running;
    }
    
    int getPid() const{
        return this->pId;
    }
    string getName() const{
        return this->name;
    }
    int getArrivalTime() const{
        return this->arrivalTime;
    }
    int getBurstTime() const{
        return this->burstTime;
    }
    int getPriority() const{
        return this->priority;
    }
    int getRemainingTime() const{
        return this->remainingTime;
    }
    int getWaitingTime() const{
        return this->waitingTime;
    }
    int getTurnAroundTime() const{
        return this->turnAroundTime;
    }
    int getCompletionTime() const{
        return this->completionTime;
    }
    
    void setRemainingTime(int time){
        this->remainingTime = time;
    }
    void setWaitingTime(int time){
        this->waitingTime = time;
    }
    void setTurnAroundTime(int time){
        this->turnAroundTime = time;
    }
    void setCompletionTime(int time){
        this->completionTime = time;
    }
    bool getRunningState(){
        return this->running;
    }
    void setRunningState(bool state){
        this->running = state;
    }
};

#endif