#include "shell.h"

constexpr size_t CompilerBKDRHash(const char *str, size_t ret= 0)
{
    return *str ? CompilerBKDRHash(str+1, (ret*HASH_SEED)+*str) : ret;
}

size_t CalcBKDRHash(const std::string &str)
{
    return CompilerBKDRHash(str.c_str());
}

std::string InvHash(size_t hash_num)
{
    std::string origin;
    while (hash_num){
        origin.insert(origin.begin(), char(hash_num%HASH_SEED));
        hash_num/= HASH_SEED;
    }

    return origin;
}

// inline size_t CalcBKDRHash(const char *str)
// {
//     size_t hash= 0;

//     while (*str){
//         hash= hash*HASH_SEED+(*str++);
//     }

//     return hash & 0x7fffffff;
// }

constexpr size_t operator ""_Hash(const char *str, size_t sz)
{
    return CompilerBKDRHash(str);
}

Shell::Shell()
: running_(NULL), creation_tree_(NULL)
{
    init();
}

Shell::~Shell()
{
    destroyCProc(creation_tree_);
    creation_tree_= NULL;
    delete [] resource_pool_.front();
    
    // prepare for possible extension
    // for (std::list<RCB*>::iterator rIter= resource_pool_.begin();
    // resource_pool_.end()!= rIter; ++rIter){
    //     delete *rIter;
    //     *rIter= NULL;
    // }
    // resource_pool_.clear();
}

PCB* Shell::getProcByPid(const int pid, PCB* rt)
{
    if (pid== rt->getPid()){
        return rt;
    }
    if (rt->c_pcb_l_.empty()){
        return NULL;
    }

    PCB* matched_pcb= NULL;
    for (std::list<PCB*>::iterator c_iter= rt->c_pcb_l_.begin();
    rt->c_pcb_l_.end()!= c_iter; ++c_iter){
        matched_pcb= getProcByPid(pid, *c_iter);
        if (NULL!= matched_pcb){
            break;
        }
    }

    return matched_pcb;
}

RCB* Shell::getResrcByRid(const int rid)
{
    RCB* matched_rcb= NULL;
    for (std::list<RCB*>::iterator r_iter= resource_pool_.begin();
    resource_pool_.end()!= r_iter; ++r_iter){
        if (rid== (*r_iter)->getRid()){
            matched_rcb= *r_iter;
            break;
        }
    }

    return matched_rcb;
}

int Shell::allocResrc(PCB* req_p, RCB* need_r, int amt)
{
    if (amt<= 0){
        std::cerr<<"error: the amount process requested should be positive"<<std::endl;
        return ALLOC_ERROR;
    } 
    if (READY== req_p->getState()){
        std::cerr<<"error: ready process can't be waken up"<<std::endl;
        return ALLOC_ERROR;
    }
    if (!checkInPool(need_r->getRid())){
        std::cerr<<"error: resource requested is not in pool"<<std::endl;
        return ALLOC_ERROR;
    }

    if (need_r->getAvailable()< amt){
        if (RUNNING== req_p->getState()){
            req_p->setState(BLOCK);
            priority_block_.push_back(req_p);
        }
        else{
            std::cerr<<"error: blocked process can't apply for resource"<<std::endl;
            return ALLOC_ERROR;
        }
        req_p->setWaitRid(need_r->getRid()); 
        req_p->setWaitRNum(amt);

        need_r->inLine(req_p);
    }
    else{
        req_p->changeResourceNum(need_r->getRid(), amt);
        need_r->changeAvailable(-amt);
    }

    return req_p->getState();
}

int Shell::releaseResrc(PCB* re_p, RCB* work_r, int amt)
{
    if (amt< 0){
        std::cerr<<"error: the amount process release should be non-negative"<<std::endl;
        return REL_ERROR;
    }
    if (re_p->getResourceNum(work_r->rid_)< amt){
        std::cerr<<"error: too much resource for process to release"<<std::endl;
        return REL_ERROR;
    }
    if (!checkInPool(work_r->rid_)){
        std::cerr<<"error: resource released is not in pool"<<std::endl;
        return REL_ERROR;
    }

    re_p->changeResourceNum(work_r->rid_, -amt);
    work_r->changeAvailable(amt);

    int wak= WAKE_NONE;
    while (!work_r->waitEmpty()){
        PCB* w_front= work_r->waitFront();
        int w_pri= w_front->priority_;
        int wait_r_num= w_front->wait_rnum_;
        if (work_r->getAvailable() < wait_r_num){
            break;
        }

        for (std::list<PCB*>::iterator b_iter= priority_block_.begin();
        priority_block_.end()!= b_iter; ++b_iter){                
            if (w_front== *b_iter){
                priority_block_.erase(b_iter);
                break;
            }
        }
        work_r->outLine(w_front);
        allocResrc(w_front, work_r, wait_r_num);

        w_front->setState(READY);
        priority_ready_[w_pri].push_back(w_front);
        w_front->setWaitRid(WAIT_NONE);
        w_front->setWaitRNum(WAIT_NONE);
        wak= WAKEN;
    }

    return wak;
}

void Shell::releaseAllResrc(PCB* proc)
{
    if (WAIT_NONE!= proc->wait_rid_){
        RCB* wait_r= getResrcByRid(proc->wait_rid_);
        wait_r->outLine(proc);
        proc->setWaitRid(WAIT_NONE);
        proc->setWaitRNum(WAIT_NONE);
    }

    for (std::map<int, int>::iterator r_iter= proc->resources_.begin();
    proc->resources_.end()!= r_iter; ++r_iter){
        RCB* work_r= getResrcByRid(r_iter->first);
        if (r_iter->second > 0){
            work_r->changeAvailable(r_iter->second);
            r_iter->second= 0;
        }
    }
}

int Shell::checkInPool(const int rid)
{
    int in_pool= false;
    for (std::list<RCB*>::iterator p_iter= resource_pool_.begin();
    resource_pool_.end()!= p_iter; ++p_iter){
        if (rid== (*p_iter)->getRid()){
            in_pool= true;
            break;
        }
    }

    return in_pool;
}

void Shell::destroyCProc(PCB* c_proc)
{
    if (0!= c_proc->getChildrenNum()){
        for (std::list<PCB*>::iterator c_iter= c_proc->c_pcb_l_.begin();
        c_proc->c_pcb_l_.end()!= c_iter; ){
            destroyCProc(*c_iter++);
        }
    }
    releaseAllResrc(c_proc);

    PCB* parent_d= c_proc->p_pcb_;
    if (NULL== parent_d){
        std::cerr<<"error: dying process don't have a proper father"<<std::endl;
        exit(1);
    }
    parent_d->killChild(c_proc);

    int d_pri= c_proc->getPriority();
    if (READY== c_proc->getState()){
        for (std::list<PCB*>::iterator r_iter= priority_ready_[d_pri].begin();
        priority_ready_[d_pri].end()!= r_iter; ++r_iter){
            if (c_proc== *r_iter){
                priority_ready_[d_pri].erase(r_iter);
                break;
            }
        }
    }
    else if (BLOCK== c_proc->getState()){
        for (std::list<PCB*>::iterator b_iter= priority_block_.begin();
        priority_block_.end()!= b_iter; ++b_iter){
            if (c_proc== *b_iter){
                priority_block_.erase(b_iter);
                break;
            }
        }
    }
    else if (RUNNING== c_proc->getState()){
        running_= NULL;
    }
    else{
        std::cerr<<"error: unexpected error while destroy a process"<<std::endl;
        exit(1);
    }
    delete c_proc;
}

void Shell::timeOut()
{
    running_->setState(READY);
    priority_ready_[running_->getPriority()].push_back(running_);
    scheduler();
}

void Shell::preempt(PCB* candidate)
{
    int c_pri= candidate->priority_;

    if (NULL!= running_ && RUNNING== running_->state_){
        int r_pri= running_->priority_;
        running_->setState(READY);
        priority_ready_[r_pri].push_back(running_);
    }
    priority_ready_[c_pri].erase(priority_ready_[c_pri].begin());
    running_= candidate;
    running_->setState(RUNNING);
}

void Shell::scheduler()
{
    // TODO
    PCB* candidate= NULL;
    int c_pri= -1;
    if (!priority_ready_[SYSTEM].empty()){
        c_pri= SYSTEM;
    }
    else if (!priority_ready_[USER].empty()){
        c_pri= USER;
    }
    else if (!priority_ready_[INIT].empty()){
        c_pri= INIT;
    }
    candidate= priority_ready_[c_pri].front();

    if ( NULL== running_
    || running_->getPriority() < candidate->getPriority()
    || RUNNING!= running_->state_){
        preempt(candidate);
    }

}

void Shell::shellRunning(int argc, char *argv[])
{
    std::string cmd_list;
    if (1== argc){
        while(std::getline(std::cin, cmd_list)){
            if (cmd_list.empty() || NULL== creation_tree_){
                return;
            }
            exec(cmd_list);
        }
    }
    else if (2== argc){
        std::ifstream input(argv[1]);
        if (!input){
            std::cout<<"usage????"<<std::endl;
            exit(1);
        }
        while(std::getline(input, cmd_list)){
            if (cmd_list.empty() || NULL== creation_tree_){
                return;
            }
            exec(cmd_list);
        }
    }

}

void Shell::exec(std::string cmd_list)
{
    std::vector<std::string> tokens;
    // tokens= split2Token(cmd_list);
    std::istringstream split_tool(cmd_list);
    std::string token;
    while (split_tool>>token){
        tokens.push_back(token);
    }

    switch(CalcBKDRHash(tokens[0])){
        case "init"_Hash:
            break;
        case "cr"_Hash:
            if (CR_LTH!= tokens.size()){
                std::cerr<<"format error: -cr <name> <priority>(=1 or 2)"<<std::endl;
                return;
            }
            createProc(tokens[1], std::stoi(tokens[2]));
            std::cout<<InvHash(running_->getPid())<<" ";
            break;
        case "de"_Hash:
            if (DE_LTH!= tokens.size()){
                std::cerr<<"format error: de <name>"<<std::endl;
                return;
            }
            destroyProc(tokens[1]);
            std::cout<<InvHash(running_->getPid())<<" ";
            break;
        case "req"_Hash:
            if (REQ_LTH!= tokens.size()){
                std::cerr<<"format error: req <resource name> <# of units>"<<std::endl;
                return;
            }
            reqResrc(tokens[1], tokens[2]);
            std::cout<<InvHash(running_->getPid())<<" ";
            break;
        case "rel"_Hash:
            if (REQ_LTH!= tokens.size()){
                std::cerr<<"format error: rel <resource name> <# of units>"<<std::endl;
                return;
            }
            relResrc(tokens[1], tokens[2]);
            std::cout<<InvHash(running_->getPid())<<" ";
            break;
        case "to"_Hash:
            if (TO_LTH!= tokens.size()){
                std::cerr<<"format error: to"<<std::endl;
                return;
            }
            timeOut();
            std::cout<<InvHash(running_->getPid())<<" ";
            break;
        case "lp"_Hash:
            if (LP_LTH!= tokens.size()){
                std::cerr<<"format error: lp"<<std::endl;
            }
            listProc();
            break;
        case "lr"_Hash:
            if (LR_LTH!= tokens.size()){
                std::cerr<<"format error: lr"<<std::endl;
            }
            listResrc();
            break;
        default:
            break;
    }
}

void Shell::init()
{
    if (NULL!= creation_tree_){
        delete creation_tree_;
        delete [] resource_pool_.front();
    }
    creation_tree_= new PCB("init"_Hash, INIT, RUNNING, NULL);
    running_= creation_tree_;
    RCB* init_resource= new RCB[4]{RCB("R1"_Hash, 1), RCB("R2"_Hash, 2), RCB("R3"_Hash, 3), RCB("R4"_Hash, 4)};
    for (int i= 0; i< 4; ++i){
        resource_pool_.push_back(init_resource+i);
    }
    std::cout<<"init ";
}
/**
 * handle cr
 */
void Shell::createProc(std::string &process_name, int priority= USER)
{
    if (USER!= priority && SYSTEM!= priority){ 
        std::cerr<<"error: bad priority"<<std::endl;
        exit(1);
    }
    size_t c_pid= CalcBKDRHash(process_name);
    if (NULL!= getProcByPid(c_pid, creation_tree_)){
        std::cerr<<"error: process has existed"<<std::endl;
        exit(1);
    }

    PCB* c_process= new PCB(c_pid, priority, READY, NULL);
    running_->addChild(c_process);
    c_process->setParent(running_);
    priority_ready_[priority].push_back(c_process);
    scheduler();
}

/**
 * handle de
 */
void Shell::destroyProc(std::string &process_name)
{
    size_t d_pid= CalcBKDRHash(process_name);
    PCB* d_pcb= getProcByPid(d_pid, creation_tree_);
    if (NULL== d_pcb){
        std::cerr<<"error: non existed process can't be destroyed"<<std::endl;
        exit(1);
    }
    
    // TODO
    destroyCProc(d_pcb);
    if (creation_tree_== d_pcb){
        creation_tree_= NULL;
        return;
    }

    for (std::list<PCB*>::iterator b_iter= priority_block_.begin();
    priority_block_.end()!= b_iter; ){
        RCB* need_r= getResrcByRid((*b_iter)->wait_rid_);
        if (need_r->available_>= (*b_iter)->wait_rnum_){
            allocResrc(*b_iter, need_r,(*b_iter)->wait_rnum_);
            need_r->outLine(*b_iter);

            (*b_iter)->wait_rid_= WAIT_NONE;
            (*b_iter)->wait_rnum_= WAIT_NONE;
            (*b_iter)->setState(READY);
            priority_ready_[(*b_iter)->priority_].push_back(*b_iter);
            priority_block_.erase(b_iter++);
        }
        else{
            ++b_iter;
        }
    }

    scheduler();
}

void Shell::reqResrc(std::string &resrc, std::string &s_amt)
{
    int rid= CalcBKDRHash(resrc);
    int amt= std::stoi(s_amt);
    RCB* need_r= getResrcByRid(rid);
    if(BLOCK== allocResrc(running_, need_r, amt)){
        scheduler();
    }
}

void Shell::relResrc(std::string &resrc, std::string &s_amt)
{
    int rid= CalcBKDRHash(resrc);
    int amt= std::stoi(s_amt);
    RCB* work_r= getResrcByRid(rid);
    if(WAKEN== releaseResrc(running_, work_r, amt)){
        scheduler();
    }
}

void Shell::listProc()
{
    std::cout<<"\n********************************************"<<std::endl;
    std::cout<<"Running Process: "<<InvHash(running_->pid_)<<std::endl;
    std::cout<<"--------------------------------------------"<<std::endl;

    std::cout<<"Ready Process:"<<std::endl;
    for (int pri= INIT; pri<= SYSTEM; ++pri){
        std::string s_pri("INIT: ");
        if (USER== pri){
            s_pri= "USER: ";
        }
        else if (SYSTEM== pri){
            s_pri= "SYSTEM: ";
        }
        std::cout<<s_pri;

        for (std::list<PCB*>::iterator r_iter= priority_ready_[pri].begin();
        priority_ready_[pri].end()!= r_iter; ++r_iter){
            std::cout<<InvHash((*r_iter)->pid_)<<" ";
        }
        putchar('\n');
    }

    std::cout<<"Blocked Process: ";
    for (std::list<PCB*>::iterator b_iter= priority_block_.begin();
    priority_block_.end()!= b_iter; ++b_iter){
        std::cout<<InvHash((*b_iter)->pid_)<<" ";
    }
    std::cout<<"\n********************************************"<<std::endl;
}

void Shell::listResrc()
{
    std::cout<<"\n********************************************"<<std::endl;
    std::cout<<"Resources: "<<std::endl;

    for (std::list<RCB*>::iterator r_iter= resource_pool_.begin();
    resource_pool_.end()!= r_iter; ++r_iter){
        std::cout<<"--------------------------------------------"<<std::endl;
        std::cout<<"Resource "<<InvHash((*r_iter)->rid_)<<std::endl;
        std::cout<<"available: "<<(*r_iter)->available_<<std::endl;
        std::cout<<"waiting Process: ";

        for (std::list<PCB*>::iterator w_iter= (*r_iter)->wait_l_.begin();
        (*r_iter)->wait_l_.end()!= w_iter; ++w_iter){
            std::cout<<InvHash((*w_iter)->pid_)<<" ";
        }
        putchar('\n');
    }

    std::cout<<"********************************************"<<std::endl;
}