#include "Scheduler.h"

Scheduler::Scheduler() {
}

vector<Process> Scheduler::getProcesses() const {
    return this->processes;
}

void Scheduler::updateProcess(int pId, const Process& process) {
    for (size_t i = 0; i < processes.size(); ++i) {
        if (processes[i].getPid() == pId) {
            processes[i] = process;
        }
    }
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

void Scheduler::clearProcesses() {
    processes.clear();
}