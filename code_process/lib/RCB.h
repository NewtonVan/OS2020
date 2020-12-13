#ifndef RCB_H
#define RCB_H

#include "hlibrary.h"

class RCB
{
    friend class Shell;
private:
    int rid_;
    int available_;
    int total_;
    std::list<PCB*> wait_l_;
public:
    RCB();
    RCB(int rid, int available, int total);
    RCB(int rid, int total);
    ~RCB();

    // get the member of RCB
    int getRid();
    int getAvailable();
    int getTotal();

    // set the member 
    int setRid(int rid);
    int setAvailable(int available);
    int setTotal(int total);
    int changeAvailable(int available);
    void inLine(PCB* wait);
    void outLine(PCB* wait);

    int waitEmpty();
    PCB* waitFront();
};

#endif