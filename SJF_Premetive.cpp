#include <iostream>
#include <vector>

#include "Process.h"
#include "Scheduler.cpp"

using namespace std;

Process SJF_NonPremetive(){
    Scheduler scheduler;

    int proccessesNum = 0;
    string pname;
    int atime;
    int btime = 0;
    int time = 0;

    cout << "How many proccesses do you wish to add? \n";
    cin >> proccessesNum;
    
    for(int i = 0; i < proccessesNum; i++){
        cout << "Proccess' name: "; cin >> pname;
        cout << "Arrival time: "; cin >> atime;
        cout << "Burst time: "; cin >> btime;

        Process newProcess = Process(pname, atime, btime);
        scheduler.addProcess(newProcess, i);
    }

    for(int i = 0; i < proccessesNum; i++){
        if(0){
            
        }
    }

}