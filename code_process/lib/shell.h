#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "RCB.h"
#include "PCB.h"

#define HASH_SEED 131

class Shell
{
private:
    PCB* running_;
    PCB* creation_tree_;
    std::list<RCB*> resource_pool_;
    std::list<PCB*> priority_ready_[3];
    std::list<PCB*> priority_block_;
public:
    Shell();
    ~Shell();

    PCB* getProcByPid(const int pid, PCB* rt);
    RCB* getResrcByRid(const int rid);
    int allocResrc(PCB* req_p, RCB* need_r, int amt);
    int releaseResrc(PCB* re_p, RCB* work_r, int amt);
    void releaseAllResrc(PCB* proc);
    int checkInPool(const int rid);
    void destroyCProc(PCB* c_proc);
    void timeOut();
    void preempt(PCB* candidate);
    void scheduler();
    void shellRunning(int argc, char *argv[]);
    void exec(std::string cmd_list);
    void init();
    void createProc(std::string &process_name, int priority);
    void destroyProc(std::string &process_name);
    void reqResrc(std::string &resrc, std::string &s_amt);
    void relResrc(std::string &resrc, std::string &s_amt);
};

constexpr size_t CompilerBKDRHash(const char *str, size_t ret);
size_t CalcBKDRHash(const std::string &str);
// constexpr size_t operator ""_Hash(const char *str, size_t sz);
std::string InvHash(size_t hash_num);

#endif