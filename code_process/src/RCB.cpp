#include "RCB.h"

RCB::RCB()
: rid_(-1), available_(0), total_(0)
{}

RCB::RCB(int rid, int available, int total)
: rid_(rid), available_(available), total_(total)
{}

RCB::RCB(int rid, int total)
: rid_(rid), available_(total), total_(total)
{}

RCB::~RCB()= default;

// get the member of RCB
int RCB::getRid()
{
    return rid_;
}

int RCB::getAvailable()
{
    return available_;
}

int RCB::getTotal()
{
    return total_;
}

// set the member
int RCB::setRid(int rid)
{
    return rid_= rid;
}

int RCB::setAvailable(int available)
{
    return available_= available;
}

int RCB::setTotal(int total)
{
    return total_= total;
}

/**
 * @available: positive means increase, negative means decrease
 */
int RCB::changeAvailable(int available)
{
    available_+= available;
    return available_;
}

void RCB::inLine(PCB* wait)
{
    wait_l_.push_back(wait);
}

void RCB::outLine(PCB* wait)
{
    for (std::list<PCB*>::iterator w_iter= wait_l_.begin();
    wait_l_.end()!= w_iter; ++w_iter){
        if (wait== *w_iter){
            wait_l_.erase(w_iter);
            break;
        }
    }
}

int RCB::waitEmpty()
{
    return wait_l_.empty();
}

PCB* RCB::waitFront()
{
    return wait_l_.front();
}