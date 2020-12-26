#ifndef PCB_H
#define PCB_H

#include "hlibrary.h"
#include <map>

class PCB
{
    friend class Shell;
private:
    int pid_;
    int priority_;
    int state_;
    int wait_rid_;
    int wait_rnum_;
    std::map<int, int> resources_;
    PCB* p_pcb_; // parent process
    std::list<PCB*> c_pcb_l_; 

public:
    PCB();
    PCB(int pid, int priority, int state, PCB* p_pcb);
    ~PCB();

    // get the member of PCB
    int getPid();
    int getPriority();
    int getState();
    int getWaitRid();
    int getWaitRNum();
    int getResourceNum(int idx);
    PCB* getParent();
    std::list<PCB*> getChildren();
    int getChildrenNum();

    // set the member of PCB
    int setPid(int pid);
    int setPriority(int priority);
    int setState(int state);
    int setWaitRid(int wait_rid);
    int setWaitRNum(int wait_rnum);
    PCB* setParent(PCB* p_pcb);
    void addChild(PCB* c_pcb);
    void killChild(PCB* c_pcb);
    void killChildren();
    void changeResourceNum(int idx, int delta);
    void reParent();

};

#endif