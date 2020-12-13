#include "PCB.h"

PCB::PCB()
: pid_(0), priority_(USER), state_(READY), wait_rid_(WAIT_NONE), wait_rnum_(WAIT_NONE), p_pcb_(NULL)
{}

PCB::PCB(int pid, int priority, int state, PCB* p_pcb)
: pid_(pid), priority_(priority), state_(state), wait_rid_(WAIT_NONE), wait_rnum_(WAIT_NONE), p_pcb_(p_pcb)
{}

PCB::~PCB()= default;

// get the member of PCB
int PCB::getPid()
{
    return pid_;
}

int PCB::getPriority()
{
    return priority_;
}

int PCB::getState()
{
    return state_;
}

int PCB::getWaitRid()
{
    return wait_rid_;
}

int PCB::getWaitRNum()
{
    return wait_rnum_;
}

/**
 * @ret: negative value mean it doesn't exists
 */
int PCB::getResourceNum(int idx)
{
    std::map<int, int>::iterator r_iter= resources_.find(idx);
    int resource_num= -1;
    if (resources_.cend()!= r_iter){
        resource_num= r_iter->second;
    }

    return resource_num;
}

PCB* PCB::getParent()
{
    return p_pcb_;
}

std::list<PCB*> PCB::getChildren()
{
    return c_pcb_l_;
}

int PCB::getChildrenNum()
{
    return c_pcb_l_.size();
}

//set the member of PCB
int PCB::setPid(int pid)
{
    return pid_= pid;
}

int PCB::setPriority(int priority)
{
    return priority_= priority;
}

int PCB::setState(int state)
{
    return state_= state;
}

int PCB::setWaitRid(int wait_rid)
{
    wait_rid_= wait_rid;
    if (WAIT_NONE== wait_rid_){
        setWaitRNum(WAIT_NONE);
    }

    return wait_rid_;
}

int PCB::setWaitRNum(int wait_rnum)
{
    return wait_rnum_= WAIT_NONE== wait_rid_ ? WAIT_NONE : wait_rnum;
}

PCB* PCB::setParent(PCB* p_pcb)
{
    return p_pcb_= p_pcb;
}

void PCB::addChild(PCB* c_pcb)
{
    c_pcb_l_.push_back(c_pcb);
}

void PCB::killChild(PCB* c_pcb)
{
    for(std::list<PCB*>::iterator c_iter= c_pcb_l_.begin(); 
    c_pcb_l_.end()!= c_iter; ++c_iter){
        if (c_pcb== *c_iter){
            c_pcb_l_.erase(c_iter);
            break;
        }
    }
}

void PCB::killChildren()
{
    c_pcb_l_.clear();
}

/**
 * @delta: positive means increase, negative means decrease
 */
void PCB::changeResourceNum(int idx, int delta)
{
    auto in_ret= resources_.insert(std::map<int, int>::value_type(idx, delta));
    if (!in_ret.second){
        in_ret.first->second+= delta;
    }
}

void PCB::reParent()
{
    if (NULL== p_pcb_){
        return;
    }

    for (std::list<PCB*>::iterator c_iter= c_pcb_l_.begin();
    c_pcb_l_.end()!= c_iter; ++c_iter){
        (*c_iter)->setParent(p_pcb_);
        p_pcb_->addChild(*c_iter);
    }
    
    killChildren();
}